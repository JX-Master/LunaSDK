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
#include "DatabaseImpl.hpp"
#include "AssetLoader.hpp"
#include "AssetType.hpp"
#include "Asset.meta.generated.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Log.hpp>
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
            !register_error_code(Asset::E_META_FILE_NOT_FOUND, "meta_file_not_found", "The asset metadata record was not found.") ||
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
        Ref<IMutex> g_metadata_mutex;
        Vector<Ref<IAssetDatabase>> g_databases;
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

        static RV validate_asset_reconciliation(AssetEntry* entry, const AssetMetaFile& file,
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

            return ok;
        }

        static RV reconcile_asset_meta_file(AssetEntry* entry, const AssetMetaFile& file,
            bool path_changes, bool allow_maintenance = false)
        {
            auto valid = validate_asset_reconciliation(entry, file, path_changes, allow_maintenance);
            if(failed(valid)) return valid;
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
            g_metadata_mutex = new_mutex();
            g_asset_registry_closing = false;
        }

        void close_asset_registry()
        {
            g_assets.clear();
            g_assets.shrink_to_fit();
            g_asset_path_mapping.clear();
            g_asset_path_mapping.shrink_to_fit();
            g_databases.clear();
            g_databases.shrink_to_fit();
        }

        static IAssetDatabase* find_database(const Path& path)
        {
            for(auto& database : g_databases)
                if(database_contains(database->get_root(), path)) return database.get();
            return nullptr;
        }

        static R<Ref<IAssetDatabase>> resolve_database(const Path& path)
        {
            if(auto database = find_database(path)) return Ref<IAssetDatabase>(database);
            Path root;
            root.root() = path.root();
            root.flags() = path.flags();
            return new_sidecar_database(root);
        }

        LUNA_ASSET_API R<Ref<IAssetDatabase>> get_asset_database(const Path& path)
        {
            MutexGuard guard(g_metadata_mutex);
            auto valid = validate_database_path(path, false, true);
            if(failed(valid)) return valid.errcode();
            if(auto database = find_database(path)) return Ref<IAssetDatabase>(database);
            return E_NOT_FOUND;
        }

        static R<AssetMetaFile> read_metadata_at(const Path& path)
        {
            auto provider = resolve_database(path);
            if(failed(provider)) return provider.errcode();
            Path relative = database_relative_path(provider.get()->get_root(), path);
            auto record = provider.get()->read_record(relative);
            if(failed(record)) return record.errcode();
            if(record.get().path != relative) return E_BAD_DATA;
            auto valid = validate_metadata(record.get());
            if(failed(valid)) return valid.errcode();
            return metadata_to_file(record.get());
        }

        static RV check_metadata_destination(const Path& path, const Guid& guid)
        {
            auto provider = resolve_database(path);
            if(failed(provider)) return provider.errcode();
            if(provider.get()->is_read_only()) return E_NOT_SUPPORTED;
            auto relative = database_relative_path(provider.get()->get_root(), path);
            if(provider.get()->is_metadata_file(relative)) return E_BAD_ARGUMENTS;
            auto record = provider.get()->read_record(relative);
            if(succeeded(record)) return record.get().guid == guid ? RV(ok) : RV(E_ALREADY_EXISTS);
            return unwrap_errcode(record.errcode()) == E_META_FILE_NOT_FOUND ? RV(ok) : RV(record.errcode());
        }

        static RV write_metadata_at(const AssetMetaFile& file, const Path& path,
            const Path& previous_path = Path(), IAssetDatabase* previous_database = nullptr)
        {
            lutry
            {
                lulet(target, resolve_database(path));
                if(target->is_read_only() || (previous_database && previous_database->is_read_only())) return E_NOT_SUPPORTED;
                Path relative = database_relative_path(target->get_root(), path);
                auto record = metadata_from_file(file, relative);
                bool same = previous_database == find_database(path) &&
                    (previous_path.empty() || database_contains(target->get_root(), previous_path));
                if(same)
                {
                    return target->write_record(record, previous_path.empty() ? Path() :
                        database_relative_path(target->get_root(), previous_path));
                }
                auto old_target = target->read_record(relative);
                if(failed(old_target) && unwrap_errcode(old_target.errcode()) != E_META_FILE_NOT_FOUND) return old_target.errcode();
                luexp(target->write_record(record));
                if(!previous_path.empty())
                {
                    auto source_result = previous_database ? R<Ref<IAssetDatabase>>(Ref<IAssetDatabase>(previous_database)) : resolve_database(previous_path);
                    RV removed = source_result.valid() ? source_result.get()->remove_record(
                        database_relative_path(source_result.get()->get_root(), previous_path)) : RV(source_result.errcode());
                    if(failed(removed) && unwrap_errcode(removed.errcode()) != E_META_FILE_NOT_FOUND)
                    {
                        Error error = removed.errcode() == E_ERROR_OBJECT ? get_error() : Error(removed.errcode(), String(explain(removed.errcode())));
                        auto restored = succeeded(old_target) ? target->write_record(old_target.get()) : target->remove_record(relative);
                        if(failed(restored)) error.message.append("; failed to restore destination metadata");
                        get_error() = move(error);
                        return E_ERROR_OBJECT;
                    }
                }
            }
            lucatchret;
            return ok;
        }

        static RV remove_metadata_at(const Path& path, IAssetDatabase* database)
        {
            if(path.empty()) return ok;
            auto provider = database ? R<Ref<IAssetDatabase>>(Ref<IAssetDatabase>(database)) : resolve_database(path);
            if(failed(provider)) return provider.errcode();
            auto result = provider.get()->remove_record(database_relative_path(provider.get()->get_root(), path));
            return failed(result) && unwrap_errcode(result.errcode()) == E_META_FILE_NOT_FOUND ? RV(ok) : result;
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
            MutexGuard metadata_guard(g_metadata_mutex);
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if(entry->maintenance || g_asset_registry_closing) return E_ASSET_DATA_UNIT_BUSY;
            if(!entry->type.empty()) return Asset::E_ASSET_ALREADY_REGISTERED;
            entry->type = type;
            ++entry->main_data_unit.revision;
            return ok;
        }

        LUNA_ASSET_API R<asset_t> new_asset(const Path& path, const Name& type, bool save_meta)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(g_asset_registry_closing) return E_ASSET_NOT_REGISTERED;
            if(type.empty()) return E_BAD_ARGUMENTS;
            if(save_meta && path.empty()) return E_EMPTY_ASSET_PATH;
            auto valid = validate_database_path(path, false, true);
            if(failed(valid)) return valid.errcode();
            {
                MutexGuard guard(g_assets_mutex);
                auto found = g_asset_path_mapping.find(path);
                if(!path.empty() && found != g_asset_path_mapping.end()) return found->second;
            }
            auto database = path.empty() ? nullptr : find_database(path);
            if(database && database->is_read_only()) return E_NOT_SUPPORTED;
            asset_t asset = get_asset();
            AssetEntry* entry = (AssetEntry*)asset.handle;
            if(save_meta)
            {
                AssetMetaFile file;
                file.format_version = ASSET_META_FORMAT_VERSION;
                file.guid = entry->guid;
                file.type = type;
                auto result = write_metadata_at(file, path);
                if(failed(result)) return result.errcode();
            }
            MutexGuard guard(g_assets_mutex);
            LockGuard entry_guard(entry->lock);
            entry->path = path;
            entry->metadata_path = save_meta ? path : Path();
            entry->database = database;
            entry->type = type;
            ++entry->main_data_unit.revision;
            if(!path.empty()) g_asset_path_mapping.insert(make_pair(path, asset));
            return asset;
        }

        struct AssetMetaUpdateInfo
        {
            Path path;
            AssetMetaFile meta_file;
            IAssetDatabase* database = nullptr;
        };

        struct MetadataReservations
        {
            Vector<AssetEntry*> entries;
            ~MetadataReservations() { for(auto entry : entries) end_asset_maintenance(entry); }
        };

        static bool has_loaded_data(AssetEntry* entry)
        {
            if(entry->main_data_unit.data) return true;
            for(const auto& unit : entry->data_units) if(unit.second.data) return true;
            return false;
        }

        static void clear_asset_registration(AssetEntry* entry)
        {
            if(!entry->path.empty()) g_asset_path_mapping.erase(entry->path);
            entry->type.reset();
            entry->path.clear();
            entry->metadata_path.clear();
            entry->database = nullptr;
            entry->data_units.clear();
            entry->main_data_unit.data = nullptr;
            ++entry->main_data_unit.revision;
        }

        // The metadata mutex serializes registry ownership changes. Entry reservations bridge
        // external provider I/O and commit without holding registry or spin locks during I/O.
        static RV apply_metadata_batch(const Vector<AssetMetaUpdateInfo>& updates, bool allow_overwrite,
            IAssetDatabase* reload = nullptr, Span<const AssetMetadata> snapshot = {})
        {
            if(g_asset_registry_closing) return E_ASSET_NOT_REGISTERED;
            HashSet<Guid> guids;
            HashSet<Path> paths;
            for(const auto& info : updates)
            {
                auto valid = validate_metadata(metadata_from_file(info.meta_file, Path("record")));
                if(failed(valid)) return valid;
                if(guids.contains(info.meta_file.guid) || paths.contains(info.path)) return E_ALREADY_EXISTS;
                guids.insert(info.meta_file.guid);
                paths.insert(info.path);
            }
            MetadataReservations reservations;
            Vector<AssetEntry*> removed;
            Vector<const AssetMetaUpdateInfo*> accepted;
            {
                MutexGuard guard(g_assets_mutex);
                for(const auto& info : updates)
                {
                    auto found = g_assets.find(info.meta_file.guid);
                    AssetEntry* entry = found == g_assets.end() ? nullptr : found->get();
                    auto mapped = g_asset_path_mapping.find(info.path);
                    if(mapped != g_asset_path_mapping.end() && (!entry || mapped->second.handle != entry)) return E_ALREADY_EXISTS;
                    if(entry)
                    {
                        LockGuard entry_guard(entry->lock);
                        if(!entry->type.empty() && entry->database != info.database) return E_ALREADY_EXISTS;
                        if(!entry->type.empty() && !allow_overwrite) continue;
                        auto valid = validate_asset_reconciliation(entry, info.meta_file, entry->path != info.path);
                        if(failed(valid)) return valid;
                        entry->maintenance = true;
                        reservations.entries.push_back(entry);
                    }
                    accepted.push_back(&info);
                }
                if(reload)
                {
                    for(auto& asset : g_assets)
                    {
                        auto entry = asset.get();
                        LockGuard entry_guard(entry->lock);
                        if(entry->database != reload || guids.contains(entry->guid)) continue;
                        if(entry->maintenance || has_asset_data_unit_operation(entry) || has_loaded_data(entry)) return E_ASSET_DATA_UNIT_BUSY;
                        entry->maintenance = true;
                        reservations.entries.push_back(entry);
                        removed.push_back(entry);
                    }
                }
            }
            if(reload)
            {
                auto result = reload->accept_snapshot(snapshot);
                if(failed(result)) return result;
            }
            MutexGuard guard(g_assets_mutex);
            for(auto entry : removed)
            {
                LockGuard entry_guard(entry->lock);
                clear_asset_registration(entry);
            }
            for(auto info : accepted)
            {
                auto asset = get_asset(info->meta_file.guid);
                auto entry = (AssetEntry*)asset.handle;
                LockGuard entry_guard(entry->lock);
                // All error conditions were checked while reserving existing entries.
                luassert_always(succeeded(reconcile_asset_meta_file(entry, info->meta_file, entry->path != info->path, true)));
                if(!entry->path.empty()) g_asset_path_mapping.erase(entry->path);
                entry->path = info->path;
                entry->metadata_path = info->path;
                entry->database = info->database;
                g_asset_path_mapping.insert_or_assign(entry->path, asset);
            }
            return ok;
        }

        static RV append_database_records(IAssetDatabase* database, const Vector<AssetMetadata>& records,
            const Path& filter, Vector<AssetMetaUpdateInfo>& updates)
        {
            auto valid = validate_records(records.cspan());
            if(failed(valid)) return valid;
            for(const auto& record : records)
            {
                if(database->is_metadata_file(record.path)) return E_BAD_DATA;
                AssetMetaUpdateInfo info;
                info.path = database_absolute_path(database->get_root(), record.path);
                if(!database_contains(filter, info.path)) continue;
                info.meta_file = metadata_to_file(record);
                info.database = database;
                updates.push_back(move(info));
            }
            return ok;
        }

        LUNA_ASSET_API RV register_asset_database(IAssetDatabase* database)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!database) return E_BAD_ARGUMENTS;
            if(g_asset_registry_closing) return E_ASSET_NOT_REGISTERED;
            Ref<IAssetDatabase> retained(database);
            Path root = database->get_root();
            lutry
            {
                luexp(validate_database_path(root, false, true));
                for(auto& existing : g_databases)
                {
                    auto other = existing->get_root();
                    if(database_contains(root, other) || database_contains(other, root)) return E_ALREADY_EXISTS;
                }
                {
                    MutexGuard guard(g_assets_mutex);
                    for(auto& asset : g_assets)
                    {
                        LockGuard entry_guard(asset->lock);
                        if(!asset->type.empty() && !asset->path.empty() && database_contains(root, asset->path)) return E_ALREADY_EXISTS;
                    }
                }
                lulet(records, database->get_records());
                Vector<AssetMetaUpdateInfo> updates;
                luexp(append_database_records(database, records, root, updates));
                luexp(apply_metadata_batch(updates, true));
                g_databases.push_back(move(retained));
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV reload_asset_database(IAssetDatabase* database)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            bool registered = false;
            for(auto& item : g_databases) if(item.get() == database) registered = true;
            if(!registered) return E_NOT_FOUND;
            if(database->is_dirty()) return E_BUSY;
            lutry
            {
                lulet(records, database->read_snapshot());
                Vector<AssetMetaUpdateInfo> updates;
                luexp(append_database_records(database, records, database->get_root(), updates));
                luexp(apply_metadata_batch(updates, true, database, records.cspan()));
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV unregister_asset_database(IAssetDatabase* database)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            usize index = 0;
            while(index < g_databases.size() && g_databases[index].get() != database) ++index;
            if(index == g_databases.size()) return E_NOT_FOUND;
            MetadataReservations reservations;
            {
                MutexGuard guard(g_assets_mutex);
                for(auto& asset : g_assets)
                {
                    auto entry = asset.get();
                    LockGuard entry_guard(entry->lock);
                    if(entry->database != database) continue;
                    if(entry->maintenance || has_asset_data_unit_operation(entry) || has_loaded_data(entry)) return E_ASSET_DATA_UNIT_BUSY;
                    entry->maintenance = true;
                    reservations.entries.push_back(entry);
                }
            }
            auto result = database->flush();
            if(failed(result)) return result;
            MutexGuard guard(g_assets_mutex);
            for(auto entry : reservations.entries)
            {
                LockGuard entry_guard(entry->lock);
                clear_asset_registration(entry);
            }
            g_databases.erase(g_databases.begin() + index);
            return ok;
        }

        LUNA_ASSET_API RV flush_asset_databases()
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            Error first;
            bool failed_any = false;
            for(auto& database : g_databases)
            {
                auto result = database->flush();
                if(failed(result) && !failed_any)
                {
                    first = result.errcode() == E_ERROR_OBJECT ? get_error() : Error(result.errcode(), String(explain(result.errcode())));
                    failed_any = true;
                }
            }
            if(failed_any) { get_error() = move(first); return E_ERROR_OBJECT; }
            return ok;
        }

        static RV scan_legacy_metadata(const Path& directory, Vector<AssetMetaUpdateInfo>& updates)
        {
            if(find_database(directory)) return ok;
            lutry
            {
                lulet(iter, VFS::open_dir(directory));
                for(; iter->is_valid(); iter->move_next())
                {
                    const c8* name = iter->get_filename();
                    if(!strcmp(name, ".") || !strcmp(name, "..")) continue;
                    Path path(directory);
                    path.push_back(name);
                    if(test_flags(iter->get_attributes(), FileAttributeFlag::directory))
                    {
                        luexp(scan_legacy_metadata(path, updates));
                    }
                    else if(path.extension() == "meta")
                    {
                        path.remove_extension();
                        if(find_database(path)) continue;
                        lulet(file, read_sidecar(path));
                        AssetMetaUpdateInfo info;
                        info.path = path;
                        info.meta_file = move(file);
                        updates.push_back(move(info));
                    }
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV load_assets_meta(const Path& path, bool allow_overwrite)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            Vector<AssetMetaUpdateInfo> updates;
            lutry
            {
                luexp(validate_database_path(path, false, true));
                if(auto database = find_database(path))
                {
                    lulet(records, database->get_records());
                    luexp(append_database_records(database, records, path, updates));
                    if(updates.empty() && path != database->get_root()) return E_META_FILE_NOT_FOUND;
                }
                else
                {
                    auto attr = VFS::get_file_attribute(path);
                    if(succeeded(attr) && test_flags(attr.get().attributes, FileAttributeFlag::directory))
                    {
                        luexp(scan_legacy_metadata(path, updates));
                        for(auto& database : g_databases)
                        {
                            if(!database_contains(path, database->get_root())) continue;
                            lulet(records, database->get_records());
                            luexp(append_database_records(database, records, path, updates));
                        }
                    }
                    else
                    {
                        lulet(file, read_metadata_at(path));
                        AssetMetaUpdateInfo info;
                        info.path = path;
                        info.meta_file = move(file);
                        updates.push_back(move(info));
                    }
                }
                luexp(apply_metadata_batch(updates, allow_overwrite));
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV load_asset_meta(asset_t asset)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!asset) return E_BAD_ARGUMENTS;
            auto entry = (AssetEntry*)asset.handle;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard reservation{entry};
                Path path = get_asset_path(asset);
                if(path.empty()) return E_EMPTY_ASSET_PATH;
                lulet(file, read_metadata_at(path));
                if(file.guid != entry->guid) return E_BAD_DATA;
                auto database = find_database(path);
                if(entry->database != database && !entry->type.empty()) return E_ALREADY_EXISTS;
                LockGuard guard(entry->lock);
                luexp(reconcile_asset_meta_file(entry, file, false, true));
                entry->metadata_path = path;
                entry->database = database;
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV save_asset_meta(asset_t asset)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!asset) return E_BAD_ARGUMENTS;
            auto entry = (AssetEntry*)asset.handle;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard reservation{entry};
                AssetMetaFile file;
                Path path, previous;
                IAssetDatabase* old_database;
                {
                    LockGuard guard(entry->lock);
                    if(entry->type.empty()) return E_ASSET_NOT_REGISTERED;
                    if(entry->path.empty()) return E_EMPTY_ASSET_PATH;
                    path = entry->path;
                    previous = entry->metadata_path;
                    old_database = entry->database;
                    file = get_asset_meta_file(entry);
                }
                luexp(write_metadata_at(file, path, previous, old_database));
                LockGuard guard(entry->lock);
                entry->metadata_path = path;
                entry->database = find_database(path);
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
            MutexGuard metadata_guard(g_metadata_mutex);
            auto valid = validate_database_path(path, false, true);
            if(failed(valid)) return valid;
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
            MutexGuard metadata_guard(g_metadata_mutex);
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
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!asset) return E_BAD_ARGUMENTS;
            Path path = get_asset_path(asset);
            if(path.empty()) return E_ASSET_NOT_REGISTERED;
            Name filename = path.back();
            auto database = find_database(path);
            path.pop_back();
            lutry
            {
                lulet(iter, VFS::open_dir(path));
                for(; iter->is_valid(); iter->move_next())
                {
                    if(test_flags(iter->get_attributes(), FileAttributeFlag::directory)) continue;
                    const c8* name = iter->get_filename();
                    Path file_path(path);
                    file_path.push_back(name);
                    if(file_path.extension() == "meta" || !strncmp(name, ".luna-asset-", 12)) continue;
                    if(database && database->is_metadata_file(database_relative_path(database->get_root(), file_path))) continue;
                    const usize size = strlen(name);
                    if(size >= filename.size() && !memcmp(name, filename.c_str(), filename.size()) &&
                        (size == filename.size() || (size > filename.size() + 1 && name[filename.size()] == '.')))
                        filenames.push_back(name);
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API RV delete_asset(asset_t asset)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!asset) return E_BAD_ARGUMENTS;
            auto entry = (AssetEntry*)asset.handle;
            Vector<ObjRef> old_data;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard reservation{entry};
                if(entry->database && entry->database->is_read_only()) return E_NOT_SUPPORTED;
                auto current_database = find_database(entry->path);
                if(current_database && current_database->is_read_only()) return E_NOT_SUPPORTED;
                Vector<Name> files;
                luexp(get_asset_files(asset, files));
                Path directory = get_asset_path(asset);
                directory.pop_back();
                for(const auto& name : files)
                {
                    Path path(directory);
                    path.push_back(name);
                    luexp(VFS::delete_file(path));
                }
                luexp(remove_metadata_at(entry->metadata_path.empty() ? entry->path : entry->metadata_path, entry->database));
                MutexGuard guard(g_assets_mutex);
                LockGuard entry_guard(entry->lock);
                if(entry->main_data_unit.data) old_data.push_back(move(entry->main_data_unit.data));
                for(auto& unit : entry->data_units) if(unit.second.data) old_data.push_back(move(unit.second.data));
                clear_asset_registration(entry);
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
                destination.append_extension(source_filename.c_str() + extension_begin);
            return destination;
        }

        static R<Vector<Pair<Path, Path>>> prepare_asset_file_transfer(asset_t source, const Path& destination)
        {
            Vector<Pair<Path, Path>> result;
            lutry
            {
                Vector<Name> files;
                luexp(get_asset_files(source, files));
                Path source_path = get_asset_path(source);
                Name source_name = source_path.back();
                Name destination_name = destination.back();
                source_path.pop_back();
                Path destination_directory(destination);
                destination_directory.pop_back();
                auto target_database = find_database(destination);
                for(const auto& name : files)
                {
                    Path from(source_path);
                    from.push_back(name);
                    Path to = get_destination_asset_file_path(destination_directory, destination_name, name, source_name.size());
                    if(target_database && target_database->is_metadata_file(database_relative_path(target_database->get_root(), to)))
                        return E_ALREADY_EXISTS;
                    auto attr = VFS::get_file_attribute(to);
                    if(succeeded(attr)) return set_error(E_ALREADY_EXISTS, "Asset destination %s already exists.", to.encode().c_str());
                    if(unwrap_errcode(attr.errcode()) != E_NOT_FOUND) return attr.errcode();
                    result.push_back(make_pair(move(from), move(to)));
                }
            }
            lucatchret;
            return result;
        }

        static RV rollback_asset_transfer(RV failure, const Vector<Pair<Path, Path>>& files, usize completed, bool moving)
        {
            Error error = failure.errcode() == E_ERROR_OBJECT ? get_error() : Error(failure.errcode(), String(explain(failure.errcode())));
            while(completed)
            {
                const auto& paths = files[--completed];
                auto result = moving ? VFS::move_file(paths.second, paths.first) : VFS::delete_file(paths.second);
                if(failed(result))
                {
                    error.message.append("; failed to restore asset file at ");
                    error.message.append(paths.second.encode());
                }
            }
            get_error() = move(error);
            return E_ERROR_OBJECT;
        }

        LUNA_ASSET_API RV move_asset(asset_t asset, const Path& new_path)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!asset) return E_BAD_ARGUMENTS;
            if(new_path.empty()) return E_EMPTY_ASSET_PATH;
            auto valid = validate_database_path(new_path, false);
            if(failed(valid)) return valid;
            auto entry = (AssetEntry*)asset.handle;
            lutry
            {
                luexp(begin_asset_maintenance(entry));
                AssetMaintenanceGuard reservation{entry};
                Path old_path = get_asset_path(asset);
                if(old_path == new_path) return ok;
                if(old_path.empty() || entry->type.empty()) return E_ASSET_NOT_REGISTERED;
                if(entry->database && entry->database->is_read_only()) return E_NOT_SUPPORTED;
                auto current_database = find_database(old_path);
                if(current_database && current_database->is_read_only()) return E_NOT_SUPPORTED;
                {
                    MutexGuard guard(g_assets_mutex);
                    auto found = g_asset_path_mapping.find(new_path);
                    if(found != g_asset_path_mapping.end() && found->second != asset) return E_ALREADY_EXISTS;
                }
                luexp(check_metadata_destination(new_path, entry->guid));
                lulet(files, prepare_asset_file_transfer(asset, new_path));
                usize completed = 0;
                for(const auto& paths : files)
                {
                    auto result = VFS::move_file(paths.first, paths.second);
                    if(failed(result)) return rollback_asset_transfer(result, files, completed, true);
                    ++completed;
                }
                AssetMetaFile metadata;
                {
                    LockGuard guard(entry->lock);
                    metadata = get_asset_meta_file(entry);
                }
                auto saved = write_metadata_at(metadata, new_path, entry->metadata_path, entry->database);
                if(failed(saved)) return rollback_asset_transfer(saved, files, completed, true);
                MutexGuard guard(g_assets_mutex);
                LockGuard entry_guard(entry->lock);
                g_asset_path_mapping.erase(old_path);
                entry->path = new_path;
                entry->metadata_path = new_path;
                entry->database = find_database(new_path);
                g_asset_path_mapping.insert_or_assign(new_path, asset);
            }
            lucatchret;
            return ok;
        }

        LUNA_ASSET_API R<asset_t> copy_asset(asset_t asset, const Path& new_path, const Guid& guid)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
            if(!asset) return E_BAD_ARGUMENTS;
            if(new_path.empty()) return E_EMPTY_ASSET_PATH;
            auto source_entry = (AssetEntry*)asset.handle;
            asset_t copied;
            lutry
            {
                luexp(validate_database_path(new_path, false));
                luexp(begin_asset_maintenance(source_entry));
                AssetMaintenanceGuard source_reservation{source_entry};
                if(source_entry->path.empty() || source_entry->type.empty()) return E_ASSET_NOT_REGISTERED;
                if(guid == source_entry->guid) return E_ALREADY_EXISTS;
                {
                    MutexGuard guard(g_assets_mutex);
                    if(g_asset_path_mapping.find(new_path) != g_asset_path_mapping.end()) return E_ALREADY_EXISTS;
                }
                copied = get_asset(guid);
                auto target = (AssetEntry*)copied.handle;
                luexp(begin_asset_maintenance(target));
                AssetMaintenanceGuard target_reservation{target};
                {
                    LockGuard guard(target->lock);
                    if(!target->type.empty() || has_loaded_data(target)) return E_ALREADY_EXISTS;
                }
                luexp(check_metadata_destination(new_path, target->guid));
                lulet(files, prepare_asset_file_transfer(asset, new_path));
                usize completed = 0;
                for(const auto& paths : files)
                {
                    auto result = VFS::copy_file(paths.first, paths.second);
                    if(failed(result)) return rollback_asset_transfer(result, files, completed, false).errcode();
                    ++completed;
                }
                AssetMetaFile metadata;
                {
                    LockGuard guard(source_entry->lock);
                    metadata = get_asset_meta_file(source_entry);
                }
                metadata.guid = target->guid;
                auto saved = write_metadata_at(metadata, new_path);
                if(failed(saved)) return rollback_asset_transfer(saved, files, completed, false).errcode();
                MutexGuard guard(g_assets_mutex);
                LockGuard target_guard(target->lock);
                luassert_always(succeeded(reconcile_asset_meta_file(target, metadata, true, true)));
                if(!target->path.empty()) g_asset_path_mapping.erase(target->path);
                target->path = new_path;
                target->metadata_path = new_path;
                target->database = find_database(new_path);
                g_asset_path_mapping.insert_or_assign(new_path, copied);
            }
            lucatchret;
            return copied;
        }

        LUNA_ASSET_API RV add_asset_data_unit(asset_t asset, const AssetDataUnitDesc& desc)
        {
            MutexGuard metadata_guard(g_metadata_mutex);
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
            MutexGuard metadata_guard(g_metadata_mutex);
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
                bool active = false;
                {
                    MutexGuard metadata_guard(g_metadata_mutex);
                    {
                        MutexGuard guard(g_assets_mutex);
                        for(auto& asset : g_assets)
                        {
                            LockGuard entry_guard(asset->lock);
                            if(asset->maintenance || has_asset_data_unit_operation(asset.get())) { active = true; break; }
                        }
                        if(!active) g_asset_registry_closing = true;
                    }
                    if(!active)
                    {
                        auto saved = flush_asset_databases();
                        if(failed(saved)) log_error("Asset", "Failed to flush metadata databases during shutdown: %s", explain(saved.errcode()));
                        MutexGuard registry_guard(g_assets_mutex);
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
                g_metadata_mutex.reset();
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
