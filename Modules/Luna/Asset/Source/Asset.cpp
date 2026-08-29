/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Asset.cpp
* @author JXMaster
* @date 2022/5/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ASSET_API LUNA_EXPORT
#include "AssetMetaFile.hpp"
#include "AssetLoader.hpp"
#include "AssetType.hpp"
#include "Asset.meta.generated.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/Runtime/SelfIndexedHashMap.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>
#include <Luna/VFS/VFS.hpp>

namespace Luna
{
    static RV register_asset_error_codes()
    {
        if(!register_error_category(Asset::ERROR_CATEGORY, "Asset") ||
            !register_error_code(Asset::E_META_FILE_NOT_FOUND, "meta_file_not_found", "The asset metadata file was not found.") ||
            !register_error_code(Asset::E_UNKNOWN_ASSET_TYPE, "unknown_asset_type", "The asset type is not registered.") ||
            !register_error_code(Asset::E_ASSET_NOT_REGISTERED, "asset_not_registered", "The asset is not registered.") ||
            !register_error_code(Asset::E_ASSET_ALREADY_REGISTERED, "asset_already_registered", "The asset is already registered.") ||
            !register_error_code(Asset::E_EMPTY_ASSET_PATH, "empty_asset_path", "The asset path is empty.") ||
            !register_error_code(Asset::E_ASSET_DATA_UNIT_NOT_LOADED,
                "asset_data_unit_not_loaded", "The specified asset data unit object has not been loaded.") ||
            !register_error_code(Asset::E_UNKNOWN_ASSET_LOADER, "unknown_asset_loader", "The asset loader is not registered.") ||
            !register_error_code(Asset::E_ASSET_DATA_UNIT_NOT_FOUND,
                "asset_data_unit_not_found", "The specified asset data unit does not exist.") ||
            !register_error_code(Asset::E_ASSET_DATA_UNIT_ALREADY_EXISTS,
                "asset_data_unit_already_exists", "An asset data unit with the specified ID already exists.") ||
            !register_error_code(Asset::E_ASSET_DATA_UNIT_BUSY, "asset_data_unit_busy", "The specified asset data unit is busy."))
        {
            return set_error(E_ALREADY_EXISTS, "Asset error metadata conflicts with an existing registration.");
        }
        return ok;
    }

    namespace Asset
    {
        constexpr u32 ASSET_META_FORMAT_VERSION = 2;

        struct AssetEntryExtractKey
        {
            Guid operator()(const UniquePtr<AssetEntry>& value)
            {
                return value->guid;
            }
        };

        Ref<IMutex> g_assets_mutex;
        SelfIndexedHashMap<Guid, UniquePtr<AssetEntry>, AssetEntryExtractKey> g_assets;
        HashMap<Path, asset_t> g_asset_path_mapping;
        bool g_asset_registry_closing = false;

        static AssetDataUnitEntry* find_asset_data_unit(AssetEntry* entry, const Name& data_unit)
        {
            if(data_unit.empty()) return &entry->main_data_unit;
            auto iter = entry->data_units.find(data_unit);
            if(iter == entry->data_units.end()) return nullptr;
            return &iter->second;
        }

        static bool has_asset_data_unit_operation(AssetEntry* entry)
        {
            if(entry->main_data_unit.operation != AssetDataUnitOperation::none) return true;
            for(auto& data_unit : entry->data_units)
            {
                if(data_unit.second.operation != AssetDataUnitOperation::none) return true;
            }
            return false;
        }

        static RV begin_asset_maintenance(AssetEntry* entry)
        {
            MutexGuard registry_guard(g_assets_mutex);
            if(g_asset_registry_closing) return Asset::E_ASSET_NOT_REGISTERED;
            LockGuard entry_guard(entry->lock);
            if(entry->maintenance || has_asset_data_unit_operation(entry))
            {
                return Asset::E_ASSET_DATA_UNIT_BUSY;
            }
            entry->maintenance = true;
            return ok;
        }

        static void end_asset_maintenance(AssetEntry* entry)
        {
            LockGuard guard(entry->lock);
            entry->maintenance = false;
        }

        struct AssetMaintenanceGuard
        {
            AssetEntry* entry = nullptr;

            ~AssetMaintenanceGuard()
            {
                if(entry) end_asset_maintenance(entry);
            }

            void dismiss()
            {
                entry = nullptr;
            }
        };

        static AssetDataUnitState get_asset_data_unit_state_internal(AssetEntry* entry, AssetDataUnitEntry* data_unit)
        {
            if(entry->type.empty()) return AssetDataUnitState::unregistered;
            if(data_unit->operation == AssetDataUnitOperation::loading) return AssetDataUnitState::loading;
            if(data_unit->data) return AssetDataUnitState::loaded;
            return AssetDataUnitState::unloaded;
        }

        static const AssetDataUnitDesc* find_asset_data_unit_desc(
            const Vector<AssetDataUnitDesc>& data_units, const Name& id)
        {
            for(const auto& data_unit : data_units)
            {
                if(data_unit.id == id) return &data_unit;
            }
            return nullptr;
        }

        static RV validate_asset_meta_file(const AssetMetaFile& file)
        {
            if(file.format_version < 1 || file.format_version > ASSET_META_FORMAT_VERSION)
            {
                return set_error(E_BAD_DATA, "Unsupported asset metadata format version %u.", file.format_version);
            }
            if(file.guid == Guid(0, 0))
            {
                return set_error(E_BAD_DATA, "Asset metadata contains an empty GUID.");
            }
            if(file.type.empty())
            {
                return set_error(E_BAD_DATA, "Asset metadata contains an empty asset type.");
            }
            for(usize i = 0; i < file.data_units.size(); ++i)
            {
                const auto& data_unit = file.data_units[i];
                if(data_unit.id.empty() || data_unit.loader.empty())
                {
                    return set_error(E_BAD_DATA, "Named asset data unit IDs and loader names must not be empty.");
                }
                for(usize j = 0; j < i; ++j)
                {
                    if(file.data_units[j].id == data_unit.id)
                    {
                        return set_error(E_BAD_DATA, "Asset metadata contains duplicate data unit ID %s.", data_unit.id.c_str());
                    }
                }
            }
            return ok;
        }

        static AssetMetaFile get_asset_meta_file(AssetEntry* entry)
        {
            AssetMetaFile file;
            file.format_version = ASSET_META_FORMAT_VERSION;
            file.guid = entry->guid;
            file.type = entry->type;
            file.data_units.reserve(entry->data_units.size());
            for(auto& data_unit : entry->data_units)
            {
                AssetDataUnitDesc desc;
                desc.id = data_unit.first;
                desc.loader = data_unit.second.loader;
                file.data_units.push_back(move(desc));
            }
            sort(file.data_units.begin(), file.data_units.end(), [](const AssetDataUnitDesc& lhs, const AssetDataUnitDesc& rhs)
            {
                return strcmp(lhs.id.c_str(), rhs.id.c_str()) < 0;
            });
            return file;
        }

        static RV reconcile_asset_meta_file(AssetEntry* entry, const AssetMetaFile& file,
            bool path_changes, bool allow_maintenance = false)
        {
            if(entry->maintenance && !allow_maintenance) return Asset::E_ASSET_DATA_UNIT_BUSY;
            if(path_changes && has_asset_data_unit_operation(entry)) return Asset::E_ASSET_DATA_UNIT_BUSY;
            if(entry->type != file.type &&
                (entry->main_data_unit.data || entry->main_data_unit.operation != AssetDataUnitOperation::none))
            {
                return Asset::E_ASSET_DATA_UNIT_BUSY;
            }
            for(auto& existing : entry->data_units)
            {
                const AssetDataUnitDesc* replacement = find_asset_data_unit_desc(file.data_units, existing.first);
                if((!replacement || replacement->loader != existing.second.loader) &&
                    (existing.second.data || existing.second.operation != AssetDataUnitOperation::none))
                {
                    return Asset::E_ASSET_DATA_UNIT_BUSY;
                }
            }

            if(entry->type != file.type)
            {
                entry->type = file.type;
                ++entry->main_data_unit.revision;
            }
            auto existing = entry->data_units.begin();
            while(existing != entry->data_units.end())
            {
                if(!find_asset_data_unit_desc(file.data_units, existing->first))
                {
                    existing = entry->data_units.erase(existing);
                }
                else
                {
                    ++existing;
                }
            }
            for(const auto& data_unit : file.data_units)
            {
                auto iter = entry->data_units.find(data_unit.id);
                if(iter == entry->data_units.end())
                {
                    AssetDataUnitEntry new_entry;
                    new_entry.loader = data_unit.loader;
                    entry->data_units.insert(make_pair(data_unit.id, move(new_entry)));
                }
                else if(iter->second.loader != data_unit.loader)
                {
                    iter->second.loader = data_unit.loader;
                    ++iter->second.revision;
                }
            }
            return ok;
        }

        void init_asset_registry()
        {
            set_serializable<AssetDataUnitDesc>();
            set_serializable<AssetMetaFile>();
            g_assets_mutex = new_mutex();
            g_asset_registry_closing = false;
        }

        void close_asset_registry()
        {
            g_assets.clear();
            g_assets.shrink_to_fit();
            g_asset_path_mapping.clear();
            g_asset_path_mapping.shrink_to_fit();
        }

        static R<AssetMetaFile> internal_load_asset_meta(const Path& meta_path)
        {
            AssetMetaFile file;
            lutry
            {
                lulet(stream, VFS::open_file(meta_path,
                    FileOpenFlag::read | FileOpenFlag::user_buffering,
                    FileCreationMode::open_existing));
                lulet(data, VariantUtils::read_json(stream));
                luexp(deserialize(file, data));
                luexp(validate_asset_meta_file(file));
            }
            lucatch
            {
                if(unwrap_errcode(luerr) == E_NOT_FOUND)
                {
                    return set_error(Asset::E_META_FILE_NOT_FOUND,
                        "Asset meta file %s is not found.", meta_path.encode().c_str());
                }
                return luerr;
            }
            return file;
        }

        static RV internal_save_asset_meta(const AssetMetaFile& file, const Path& meta_path)
        {
            lutry
            {
                lulet(data, serialize(file));
                lulet(stream, VFS::open_file(meta_path,
                    FileOpenFlag::write | FileOpenFlag::user_buffering,
                    FileCreationMode::create_always));
                luexp(VariantUtils::write_json(stream, data));
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API asset_t get_asset(const Guid& guid)
        {
            MutexGuard guard(g_assets_mutex);
            if(g_asset_registry_closing)
            {
                lucheck_msg(false, "The asset registry is closing!");
                return asset_t();
            }
            if(guid != Guid(0, 0))
            {
                auto iter = g_assets.find(guid);
                if(iter != g_assets.end()) return asset_t(iter->get());
            }
            UniquePtr<AssetEntry> entry(memnew<AssetEntry>());
            entry->guid = guid == Guid(0, 0) ? random_guid() : guid;
            asset_t asset(entry.get());
            g_assets.insert(move(entry));
            return asset;
        }

        LUNA_ASSET_API RV register_asset(asset_t asset, const Name& type)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(!entry->type.empty()) return Asset::E_ASSET_ALREADY_REGISTERED;
            entry->type = type;
            ++entry->main_data_unit.revision;
            return ok;
        }

        LUNA_ASSET_API R<asset_t> new_asset(const Path& path, const Name& type, bool save_meta_to_file)
        {
            if(save_meta_to_file && path.empty()) return Asset::E_EMPTY_ASSET_PATH;
            MutexGuard registry_guard(g_assets_mutex);
            if(!path.empty())
            {
                auto iter = g_asset_path_mapping.find(path);
                if(iter != g_asset_path_mapping.end()) return iter->second;
            }
            asset_t asset = get_asset();
            AssetEntry* entry = (AssetEntry*)asset.handle;
            {
                LockGuard entry_guard(entry->lock);
                entry->path = path;
                entry->type = type;
                ++entry->main_data_unit.revision;
                if(save_meta_to_file)
                {
                    AssetMetaFile file = get_asset_meta_file(entry);
                    Path meta_path = path;
                    meta_path.append_extension("meta");
                    entry_guard.unlock();
                    auto result = internal_save_asset_meta(file, meta_path);
                    if(failed(result)) return result.errcode();
                }
            }
            if(!path.empty()) g_asset_path_mapping.insert(make_pair(path, asset));
            return asset;
        }

        struct AssetMetaUpdateInfo
        {
            Path path;
            AssetMetaFile meta_file;
        };

        static RV recursive_load_asset_meta(const Path& directory, Vector<AssetMetaUpdateInfo>& assets)
        {
            lutry
            {
                lulet(iter, VFS::open_dir(directory));
                Path path = directory;
                for(; iter->is_valid(); iter->move_next())
                {
                    const c8* filename = iter->get_filename();
                    if(!strcmp(filename, ".") || !strcmp(filename, "..")) continue;
                    path.push_back(filename);
                    if(test_flags(iter->get_attributes(), FileAttributeFlag::directory))
                    {
                        luexp(recursive_load_asset_meta(path, assets));
                    }
                    else if(path.extension() == "meta")
                    {
                        lulet(meta_file, internal_load_asset_meta(path));
                        AssetMetaUpdateInfo info;
                        path.remove_extension();
                        info.path = path;
                        info.meta_file = move(meta_file);
                        assets.push_back(move(info));
                    }
                    path.pop_back();
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV load_assets_meta(const Path& path, bool allow_overwrite)
        {
            MutexGuard registry_guard(g_assets_mutex);
            Vector<AssetMetaUpdateInfo> update_assets;
            lutry
            {
                auto attributes = VFS::get_file_attribute(path);
                if(succeeded(attributes) && test_flags(attributes.get().attributes, FileAttributeFlag::directory))
                {
                    luexp(recursive_load_asset_meta(path, update_assets));
                }
                else
                {
                    Path meta_path = path;
                    meta_path.append_extension("meta");
                    lulet(meta_file, internal_load_asset_meta(meta_path));
                    AssetMetaUpdateInfo info;
                    info.path = path;
                    info.meta_file = move(meta_file);
                    update_assets.push_back(move(info));
                }

                for(auto& info : update_assets)
                {
                    asset_t asset = get_asset(info.meta_file.guid);
                    AssetEntry* entry = (AssetEntry*)asset.handle;
                    LockGuard entry_guard(entry->lock);
                    if(!entry->type.empty() && !allow_overwrite) continue;
                    auto mapped = g_asset_path_mapping.find(info.path);
                    if(mapped != g_asset_path_mapping.end() && mapped->second != asset)
                    {
                        luthrow(set_error(E_ALREADY_EXISTS,
                            "Another asset is already registered at %s.", info.path.encode().c_str()));
                    }
                    const bool path_changes = entry->path != info.path;
                    luexp(reconcile_asset_meta_file(entry, info.meta_file, path_changes));
                    if(path_changes)
                    {
                        if(!entry->path.empty())
                        {
                            auto old_mapping = g_asset_path_mapping.find(entry->path);
                            if(old_mapping != g_asset_path_mapping.end() && old_mapping->second == asset)
                            {
                                g_asset_path_mapping.erase(old_mapping);
                            }
                        }
                        entry->path = info.path;
                    }
                    g_asset_path_mapping.insert_or_assign(entry->path, asset);
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV load_asset_meta(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard maintenance_guard{entry};
                Path path;
                {
                    LockGuard guard(entry->lock);
                    path = entry->path;
                }
                if(path.empty())
                {
                    luthrow(set_error(E_BAD_ARGUMENTS,
                        "Asset::load_asset_meta: Asset path is not valid."));
                }
                Path meta_path = path;
                meta_path.append_extension("meta");
                lulet(meta_file, internal_load_asset_meta(meta_path));
                LockGuard guard(entry->lock);
                if(entry->guid != meta_file.guid)
                {
                    luthrow(set_error(E_BAD_DATA,
                        "The asset GUID loaded from the metadata file does not match the registered asset GUID."));
                }
                if(entry->path != path) luthrow(Asset::E_ASSET_DATA_UNIT_BUSY);
                luexp(reconcile_asset_meta_file(entry, meta_file, false, true));
                guard.unlock();
                end_asset_maintenance(entry);
                maintenance_guard.dismiss();
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV save_asset_meta(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard maintenance_guard{entry};
                Path path;
                AssetMetaFile file;
                {
                    LockGuard guard(entry->lock);
                    if(entry->path.empty())
                    {
                        luthrow(set_error(E_BAD_ARGUMENTS,
                            "Asset::save_asset_meta: Asset path is not valid."));
                    }
                    if(entry->type.empty()) luthrow(Asset::E_ASSET_NOT_REGISTERED);
                    path = entry->path;
                    file = get_asset_meta_file(entry);
                }
                path.append_extension("meta");
                luexp(internal_save_asset_meta(file, path));
                end_asset_maintenance(entry);
                maintenance_guard.dismiss();
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API R<asset_t> get_asset_by_path(const Path& path)
        {
            MutexGuard guard(g_assets_mutex);
            auto iter = g_asset_path_mapping.find(path);
            if(iter != g_asset_path_mapping.end()) return iter->second;
            return E_NOT_FOUND;
        }

        LUNA_ASSET_API Guid get_asset_guid(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            return entry->guid;
        }

        LUNA_ASSET_API Path get_asset_path(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            return entry->path;
        }

        LUNA_ASSET_API RV set_asset_path(asset_t asset, const Path& path)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            MutexGuard registry_guard(g_assets_mutex);
            LockGuard entry_guard(entry->lock);
            if(entry->maintenance || has_asset_data_unit_operation(entry)) return Asset::E_ASSET_DATA_UNIT_BUSY;
            if(entry->path == path) return ok;
            if(!path.empty())
            {
                auto iter = g_asset_path_mapping.find(path);
                if(iter != g_asset_path_mapping.end() && iter->second != asset) return E_ALREADY_EXISTS;
            }
            if(!entry->path.empty()) g_asset_path_mapping.erase(entry->path);
            entry->path = path;
            if(!path.empty()) g_asset_path_mapping.insert_or_assign(path, asset);
            return ok;
        }

        LUNA_ASSET_API Name get_asset_name(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(entry->path.empty()) return Name();
            return entry->path.filename();
        }

        LUNA_ASSET_API Name get_asset_type(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            return entry->type;
        }

        LUNA_ASSET_API void set_asset_type(asset_t asset, const Name& type)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            if(type.empty())
            {
                lucheck_msg(false, "Asset type must not be empty!");
                return;
            }
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(entry->maintenance ||
                entry->main_data_unit.operation != AssetDataUnitOperation::none ||
                entry->main_data_unit.data)
            {
                lucheck_msg(false, "The main asset data unit must be unloaded and idle before changing the asset type!");
                return;
            }
            if(entry->type == type) return;
            entry->type = type;
            ++entry->main_data_unit.revision;
        }

        LUNA_ASSET_API RV get_asset_files(asset_t asset, Vector<Name>& filenames)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            Path path = get_asset_path(asset);
            if(path.empty()) return Asset::E_ASSET_NOT_REGISTERED;
            Name filename = path.back();
            path.pop_back();
            lutry
            {
                lulet(iter, VFS::open_dir(path));
                while(iter->is_valid())
                {
                    if(!test_flags(iter->get_attributes(), FileAttributeFlag::directory))
                    {
                        const c8* name = iter->get_filename();
                        const usize name_size = strlen(name);
                        if(name_size >= filename.size() && !memcmp(name, filename.c_str(), filename.size()))
                        {
                            if(name_size == filename.size() ||
                                (name_size > filename.size() + 1 && name[filename.size()] == '.'))
                            {
                                filenames.push_back(name);
                            }
                        }
                    }
                    iter->move_next();
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV delete_asset(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard maintenance_guard{entry};
                Vector<Name> files;
                luexp(get_asset_files(asset, files));
                Path asset_path = get_asset_path(asset);
                Path directory = asset_path;
                directory.pop_back();
                for(auto& filename : files)
                {
                    directory.push_back(filename);
                    luexp(VFS::delete_file(directory));
                    directory.pop_back();
                }

                {
                    MutexGuard registry_guard(g_assets_mutex);
                    g_asset_path_mapping.erase(asset_path);
                }

                Vector<ObjRef> old_data;
                {
                    LockGuard entry_guard(entry->lock);
                    if(entry->main_data_unit.data) old_data.push_back(move(entry->main_data_unit.data));
                    for(auto& data_unit : entry->data_units)
                    {
                        if(data_unit.second.data) old_data.push_back(move(data_unit.second.data));
                    }
                    entry->data_units.clear();
                    entry->main_data_unit.operation = AssetDataUnitOperation::none;
                    ++entry->main_data_unit.revision;
                    entry->type.reset();
                    entry->path.clear();
                    entry->maintenance = false;
                }
                maintenance_guard.dismiss();
            }
            lucatchret;
            return ok;
        }

        static Path get_destination_asset_file_path(const Path& destination_directory,
            const Name& destination_filename, const Name& source_filename, usize source_base_name_size)
        {
            Path destination = destination_directory;
            destination.push_back(destination_filename);
            const usize extension_begin = source_base_name_size + 1;
            if(source_filename.size() > extension_begin)
            {
                destination.append_extension(source_filename.c_str() + extension_begin);
            }
            return destination;
        }

        LUNA_ASSET_API RV move_asset(asset_t asset, const Path& new_path)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            if(new_path.empty()) return Asset::E_EMPTY_ASSET_PATH;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            MutexGuard registry_guard(g_assets_mutex);
            auto mapped = g_asset_path_mapping.find(new_path);
            if(mapped != g_asset_path_mapping.end() && mapped->second != asset) return E_ALREADY_EXISTS;
            Path old_path = get_asset_path(asset);
            if(old_path == new_path) return ok;

            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard maintenance_guard{entry};
                Vector<Name> files;
                luexp(get_asset_files(asset, files));
                Name old_filename = old_path.back();
                Name new_filename = new_path.back();
                Path old_directory = old_path;
                Path new_directory = new_path;
                old_directory.pop_back();
                new_directory.pop_back();

                for(auto& filename : files)
                {
                    Path destination = get_destination_asset_file_path(
                        new_directory, new_filename, filename, old_filename.size());
                    auto attributes = VFS::get_file_attribute(destination);
                    if(succeeded(attributes))
                    {
                        luthrow(set_error(E_ALREADY_EXISTS,
                            "Cannot move asset file to %s because it already exists.", destination.encode().c_str()));
                    }
                }
                for(auto& filename : files)
                {
                    Path source = old_directory;
                    source.push_back(filename);
                    Path destination = get_destination_asset_file_path(
                        new_directory, new_filename, filename, old_filename.size());
                    luexp(VFS::move_file(source, destination));
                }

                {
                    LockGuard entry_guard(entry->lock);
                    g_asset_path_mapping.erase(entry->path);
                    entry->path = new_path;
                    g_asset_path_mapping.insert_or_assign(entry->path, asset);
                    AssetMetaFile file = get_asset_meta_file(entry);
                    Path meta_path = new_path;
                    meta_path.append_extension("meta");
                    entry_guard.unlock();
                    luexp(internal_save_asset_meta(file, meta_path));
                }
                end_asset_maintenance(entry);
                maintenance_guard.dismiss();
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API R<asset_t> copy_asset(asset_t asset, const Path& new_path, const Guid& guid)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            if(new_path.empty()) return Asset::E_EMPTY_ASSET_PATH;
            if(guid != Guid(0, 0) && guid == get_asset_guid(asset)) return E_ALREADY_EXISTS;
            AssetEntry* source_entry = (AssetEntry*)asset.handle;
            MutexGuard registry_guard(g_assets_mutex);
            if(g_asset_path_mapping.find(new_path) != g_asset_path_mapping.end()) return E_ALREADY_EXISTS;

            asset_t new_asset_handle;
            lutry
            {
                luexp(begin_asset_maintenance(source_entry));
                AssetMaintenanceGuard source_maintenance_guard{source_entry};
                Path old_path = get_asset_path(asset);
                Vector<Name> files;
                luexp(get_asset_files(asset, files));
                Name old_filename = old_path.back();
                Name new_filename = new_path.back();
                Path old_directory = old_path;
                Path new_directory = new_path;
                old_directory.pop_back();
                new_directory.pop_back();

                for(auto& filename : files)
                {
                    Path destination = get_destination_asset_file_path(
                        new_directory, new_filename, filename, old_filename.size());
                    auto attributes = VFS::get_file_attribute(destination);
                    if(succeeded(attributes))
                    {
                        luthrow(set_error(E_ALREADY_EXISTS,
                            "Cannot copy asset file to %s because it already exists.", destination.encode().c_str()));
                    }
                }
                for(auto& filename : files)
                {
                    Path source = old_directory;
                    source.push_back(filename);
                    Path destination = get_destination_asset_file_path(
                        new_directory, new_filename, filename, old_filename.size());
                    luexp(VFS::copy_file(source, destination));
                }

                AssetMetaFile source_meta;
                {
                    LockGuard source_guard(source_entry->lock);
                    source_meta = get_asset_meta_file(source_entry);
                }
                new_asset_handle = get_asset(guid);
                AssetEntry* destination_entry = (AssetEntry*)new_asset_handle.handle;
                luexp(begin_asset_maintenance(destination_entry));
                AssetMaintenanceGuard destination_maintenance_guard{destination_entry};
                {
                    LockGuard destination_guard(destination_entry->lock);
                    if(destination_entry->main_data_unit.data)
                    {
                        luthrow(Asset::E_ASSET_DATA_UNIT_BUSY);
                    }
                    for(auto& data_unit : destination_entry->data_units)
                    {
                        if(data_unit.second.data) luthrow(Asset::E_ASSET_DATA_UNIT_BUSY);
                    }
                    luexp(reconcile_asset_meta_file(destination_entry, source_meta, false, true));
                    if(!destination_entry->path.empty()) g_asset_path_mapping.erase(destination_entry->path);
                    destination_entry->path = new_path;
                    g_asset_path_mapping.insert_or_assign(new_path, new_asset_handle);
                    AssetMetaFile destination_meta = get_asset_meta_file(destination_entry);
                    Path meta_path = new_path;
                    meta_path.append_extension("meta");
                    destination_guard.unlock();
                    luexp(internal_save_asset_meta(destination_meta, meta_path));
                }
                end_asset_maintenance(destination_entry);
                destination_maintenance_guard.dismiss();
                end_asset_maintenance(source_entry);
                source_maintenance_guard.dismiss();
            }
            lucatchret;
            return new_asset_handle;
        }

        LUNA_ASSET_API RV add_asset_data_unit(asset_t asset, const AssetDataUnitDesc& desc)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            if(desc.id.empty() || desc.loader.empty()) return E_BAD_ARGUMENTS;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(entry->type.empty()) return Asset::E_ASSET_NOT_REGISTERED;
            if(entry->maintenance) return Asset::E_ASSET_DATA_UNIT_BUSY;
            if(entry->data_units.find(desc.id) != entry->data_units.end())
            {
                return Asset::E_ASSET_DATA_UNIT_ALREADY_EXISTS;
            }
            AssetDataUnitEntry data_unit;
            data_unit.loader = desc.loader;
            entry->data_units.insert(make_pair(desc.id, move(data_unit)));
            return ok;
        }

        LUNA_ASSET_API RV remove_asset_data_unit(asset_t asset, const Name& data_unit)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            if(data_unit.empty()) return E_BAD_ARGUMENTS;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(entry->type.empty()) return Asset::E_ASSET_NOT_REGISTERED;
            if(entry->maintenance) return Asset::E_ASSET_DATA_UNIT_BUSY;
            auto iter = entry->data_units.find(data_unit);
            if(iter == entry->data_units.end()) return Asset::E_ASSET_DATA_UNIT_NOT_FOUND;
            if(iter->second.data || iter->second.operation != AssetDataUnitOperation::none)
            {
                return Asset::E_ASSET_DATA_UNIT_BUSY;
            }
            entry->data_units.erase(iter);
            return ok;
        }

        LUNA_ASSET_API R<Name> get_asset_data_unit_loader(asset_t asset, const Name& data_unit)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            Name type;
            Name loader;
            {
                LockGuard guard(entry->lock);
                if(entry->type.empty()) return Asset::E_ASSET_NOT_REGISTERED;
                if(data_unit.empty())
                {
                    type = entry->type;
                }
                else
                {
                    auto iter = entry->data_units.find(data_unit);
                    if(iter == entry->data_units.end()) return Asset::E_ASSET_DATA_UNIT_NOT_FOUND;
                    loader = iter->second.loader;
                }
            }
            if(data_unit.empty())
            {
                auto type_desc = get_asset_type_desc(type);
                if(failed(type_desc)) return type_desc.errcode();
                loader = type_desc.get().main_data_unit_loader;
            }
            return loader;
        }

        LUNA_ASSET_API RV get_asset_data_units(asset_t asset, Vector<AssetDataUnitDesc>& out_data_units)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            Name type;
            Vector<AssetDataUnitDesc> named_data_units;
            {
                LockGuard guard(entry->lock);
                if(entry->type.empty()) return Asset::E_ASSET_NOT_REGISTERED;
                type = entry->type;
                named_data_units.reserve(entry->data_units.size());
                for(auto& data_unit : entry->data_units)
                {
                    AssetDataUnitDesc desc;
                    desc.id = data_unit.first;
                    desc.loader = data_unit.second.loader;
                    named_data_units.push_back(move(desc));
                }
            }
            auto type_desc = get_asset_type_desc(type);
            if(failed(type_desc)) return type_desc.errcode();
            AssetDataUnitDesc main_data_unit;
            main_data_unit.loader = type_desc.get().main_data_unit_loader;
            out_data_units.push_back(move(main_data_unit));
            sort(named_data_units.begin(), named_data_units.end(), [](const AssetDataUnitDesc& lhs, const AssetDataUnitDesc& rhs)
            {
                return strcmp(lhs.id.c_str(), rhs.id.c_str()) < 0;
            });
            for(auto& data_unit : named_data_units) out_data_units.push_back(move(data_unit));
            return ok;
        }

        struct AssetDataUnitOperationContext
        {
            AssetEntry* entry = nullptr;
            Name id;
            Name type;
            Name loader;
            AssetLoaderDesc loader_desc;
            Path path;
            ObjRef data;
            AssetDataUnitOperation operation = AssetDataUnitOperation::none;
            u64 revision = 0;
            bool skipped = false;
        };

        static bool finish_asset_data_unit_operation(const AssetDataUnitOperationContext& context);

        static R<AssetDataUnitOperationContext> begin_asset_data_unit_operation(
            asset_t asset, const Name& data_unit, AssetDataUnitOperation operation,
            bool skip_if_loaded, bool require_loaded)
        {
            if(!asset.handle) return E_BAD_ARGUMENTS;
            MutexGuard registry_guard(g_assets_mutex);
            if(g_asset_registry_closing) return Asset::E_ASSET_NOT_REGISTERED;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(entry->type.empty()) return Asset::E_ASSET_NOT_REGISTERED;
            if(entry->maintenance) return Asset::E_ASSET_DATA_UNIT_BUSY;
            AssetDataUnitEntry* data_unit_entry = find_asset_data_unit(entry, data_unit);
            if(!data_unit_entry) return Asset::E_ASSET_DATA_UNIT_NOT_FOUND;
            if(data_unit_entry->operation != AssetDataUnitOperation::none)
            {
                return Asset::E_ASSET_DATA_UNIT_BUSY;
            }
            AssetDataUnitOperationContext context;
            context.entry = entry;
            context.id = data_unit;
            context.type = entry->type;
            context.loader = data_unit_entry->loader;
            context.path = entry->path;
            context.operation = operation;
            if(skip_if_loaded && data_unit_entry->data)
            {
                context.skipped = true;
                return context;
            }
            if(require_loaded && !data_unit_entry->data) return Asset::E_ASSET_DATA_UNIT_NOT_LOADED;
            context.data = data_unit_entry->data;
            data_unit_entry->operation = operation;
            context.revision = ++data_unit_entry->revision;
            guard.unlock();
            registry_guard.unlock();

            Name loader_name = context.loader;
            if(context.id.empty())
            {
                auto type_desc = get_asset_type_desc(context.type);
                if(failed(type_desc))
                {
                    finish_asset_data_unit_operation(context);
                    return type_desc.errcode();
                }
                loader_name = type_desc.get().main_data_unit_loader;
            }
            auto loader_desc = get_asset_loader_desc(loader_name);
            if(failed(loader_desc))
            {
                finish_asset_data_unit_operation(context);
                return loader_desc.errcode();
            }

            context.loader = loader_name;
            context.loader_desc = move(loader_desc.get());
            return context;
        }

        static bool finish_asset_data_unit_operation(const AssetDataUnitOperationContext& context)
        {
            LockGuard guard(context.entry->lock);
            AssetDataUnitEntry* data_unit = find_asset_data_unit(context.entry, context.id);
            if(!data_unit || data_unit->revision != context.revision || data_unit->operation != context.operation)
            {
                return false;
            }
            data_unit->operation = AssetDataUnitOperation::none;
            ++data_unit->revision;
            return true;
        }

        static RV commit_asset_data_unit(
            const AssetDataUnitOperationContext& context, ObjRef&& new_data)
        {
            ObjRef old_data;
            {
                LockGuard guard(context.entry->lock);
                AssetDataUnitEntry* data_unit = find_asset_data_unit(context.entry, context.id);
                if(!data_unit || data_unit->revision != context.revision || data_unit->operation != context.operation)
                {
                    return Asset::E_ASSET_DATA_UNIT_BUSY;
                }
                old_data = move(data_unit->data);
                data_unit->data = move(new_data);
                data_unit->operation = AssetDataUnitOperation::none;
                ++data_unit->revision;
            }
            return ok;
        }

        LUNA_ASSET_API R<ObjRef> get_asset_data_unit_object(asset_t asset, const Name& data_unit)
        {
            if(!asset.handle) return E_BAD_ARGUMENTS;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            AssetDataUnitEntry* data_unit_entry = find_asset_data_unit(entry, data_unit);
            if(!data_unit_entry) return Asset::E_ASSET_DATA_UNIT_NOT_FOUND;
            return data_unit_entry->data;
        }

        LUNA_ASSET_API RV set_asset_data_unit_object(asset_t asset, const Name& data_unit, object_t data)
        {
            auto context_result = begin_asset_data_unit_operation(asset, data_unit,
                AssetDataUnitOperation::setting, false, false);
            if(failed(context_result)) return context_result.errcode();
            AssetDataUnitOperationContext context = move(context_result.get());
            AssetLoaderDesc loader = move(context.loader_desc);
            if(loader.on_set_asset_data_unit)
            {
                auto result = loader.on_set_asset_data_unit(
                    loader.userdata.get(), asset, data_unit, data);
                if(failed(result))
                {
                    finish_asset_data_unit_operation(context);
                    return result.errcode();
                }
            }
            ObjRef new_data(data);
            return commit_asset_data_unit(context, move(new_data));
        }

        static RV internal_load_asset_data_unit(asset_t asset, const Name& data_unit,
            bool force_reload, bool default_data)
        {
            auto context_result = begin_asset_data_unit_operation(asset, data_unit,
                AssetDataUnitOperation::loading, !force_reload, false);
            if(failed(context_result)) return context_result.errcode();
            AssetDataUnitOperationContext context = move(context_result.get());
            if(context.skipped) return ok;
            if(!default_data && context.path.empty())
            {
                finish_asset_data_unit_operation(context);
                return Asset::E_EMPTY_ASSET_PATH;
            }
            AssetLoaderDesc loader = move(context.loader_desc);
            R<ObjRef> data_result = E_NOT_SUPPORTED;
            if(default_data)
            {
                if(loader.on_load_asset_data_unit_default_data)
                {
                    data_result = loader.on_load_asset_data_unit_default_data(
                        loader.userdata.get(), asset, data_unit);
                }
            }
            else if(loader.on_load_asset_data_unit)
            {
                data_result = loader.on_load_asset_data_unit(
                    loader.userdata.get(), asset, data_unit, context.path);
            }
            if(failed(data_result))
            {
                finish_asset_data_unit_operation(context);
                return data_result.errcode();
            }
            if(!data_result.get())
            {
                finish_asset_data_unit_operation(context);
                return set_error(E_BAD_DATA, "Asset loader returned a null data object.");
            }
            ObjRef data = move(data_result.get());
            return commit_asset_data_unit(context, move(data));
        }

        LUNA_ASSET_API RV load_asset_data_unit(asset_t asset, const Name& data_unit, bool force_reload)
        {
            return internal_load_asset_data_unit(asset, data_unit, force_reload, false);
        }

        LUNA_ASSET_API RV load_asset_data_unit_default_data(asset_t asset, const Name& data_unit, bool force_reload)
        {
            return internal_load_asset_data_unit(asset, data_unit, force_reload, true);
        }

        LUNA_ASSET_API R<AssetDataUnitState> get_asset_data_unit_state(asset_t asset, const Name& data_unit)
        {
            if(!asset.handle) return E_BAD_ARGUMENTS;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            AssetDataUnitEntry* data_unit_entry = find_asset_data_unit(entry, data_unit);
            if(!data_unit_entry) return Asset::E_ASSET_DATA_UNIT_NOT_FOUND;
            return get_asset_data_unit_state_internal(entry, data_unit_entry);
        }

        LUNA_ASSET_API RV save_asset_data_unit(asset_t asset, const Name& data_unit)
        {
            auto context_result = begin_asset_data_unit_operation(asset, data_unit,
                AssetDataUnitOperation::saving, false, true);
            if(failed(context_result)) return context_result.errcode();
            AssetDataUnitOperationContext context = move(context_result.get());
            if(context.path.empty())
            {
                finish_asset_data_unit_operation(context);
                return Asset::E_EMPTY_ASSET_PATH;
            }
            AssetLoaderDesc loader = move(context.loader_desc);
            if(!loader.on_save_asset_data_unit)
            {
                finish_asset_data_unit_operation(context);
                return E_NOT_SUPPORTED;
            }
            auto result = loader.on_save_asset_data_unit(loader.userdata.get(), asset,
                data_unit, context.path, context.data.get());
            finish_asset_data_unit_operation(context);
            return result;
        }

        LUNA_ASSET_API void get_asset_data_unit_referred_assets(asset_t asset, const Name& data_unit,
            Vector<asset_t>& out_referred_assets)
        {
            auto context_result = begin_asset_data_unit_operation(asset, data_unit,
                AssetDataUnitOperation::querying_referred_assets, false, false);
            if(failed(context_result)) return;
            AssetDataUnitOperationContext context = move(context_result.get());
            AssetLoaderDesc loader = move(context.loader_desc);
            if(loader.on_get_referred_assets)
            {
                loader.on_get_referred_assets(loader.userdata.get(), asset,
                    data_unit, out_referred_assets);
            }
            finish_asset_data_unit_operation(context);
        }

        LUNA_ASSET_API void close()
        {
            while(true)
            {
                bool has_active_operations = false;
                {
                    MutexGuard assets_guard(g_assets_mutex);
                    for(auto& asset : g_assets)
                    {
                        AssetEntry* entry = asset.get();
                        LockGuard entry_guard(entry->lock);
                        if(entry->maintenance || has_asset_data_unit_operation(entry))
                        {
                            has_active_operations = true;
                            break;
                        }
                    }
                    if(!has_active_operations)
                    {
                        g_asset_registry_closing = true;
                        MutexGuard types_guard(g_asset_types_mutex);
                        MutexGuard loaders_guard(g_asset_loaders_mutex);
                        close_asset_registry();
                        close_asset_type();
                        close_asset_loader();
                        return;
                    }
                }
                yield_current_thread();
            }
        }

        struct AssetModule : public Module
        {
            virtual const c8* get_name() override
            {
                return "Asset";
            }

            virtual RV on_register() override
            {
                RV result = register_asset_error_codes();
                if(failed(result)) return result.errcode();
                return add_dependency_modules(this, {module_variant_utils(), module_vfs()});
            }

            virtual RV on_init() override
            {
                Meta::register_Asset_types();
                init_asset_type();
                init_asset_loader();
                init_asset_registry();
                SerializableTypeDesc desc;
                desc.serialize_func = [](typeinfo_t type, const void* instance) -> R<Variant>
                {
                    const asset_t* asset = (const asset_t*)instance;
                    if(!asset->handle) return Variant();
                    return serialize(Asset::get_asset_guid(*asset));
                };
                desc.deserialize_func = [](typeinfo_t type, void* instance, const Variant& data) -> RV
                {
                    lutry
                    {
                        asset_t* asset = (asset_t*)instance;
                        if(data.empty())
                        {
                            asset->handle = nullptr;
                        }
                        else
                        {
                            Guid guid;
                            luexp(deserialize(guid, data));
                            *asset = Asset::get_asset(guid);
                        }
                    }
                    lucatchret;
                    return ok;
                };
                set_serializable<asset_t>(&desc);
                return ok;
            }

            virtual void on_close() override
            {
                close();
                g_assets_mutex.reset();
                g_asset_types_mutex.reset();
                g_asset_loaders_mutex.reset();
            }
        };
    }

    LUNA_ASSET_API Module* module_asset()
    {
        static Asset::AssetModule module;
        return &module;
    }
}
