/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Asset.cpp
* @author JXMaster
* @date 2022/5/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ASSET_API LUNA_EXPORT
#include "Asset.hpp"
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/SelfIndexedHashMap.hpp>
#include "AssetType.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>

namespace Luna
{
    namespace Asset
    {
        struct AssetEntryExtractKey
        {
            Guid operator()(const UniquePtr<AssetEntry>& v)
            {
                return v->guid;
            }
        };
        Ref<IMutex> g_assets_mutex;
        SelfIndexedHashMap<Guid, UniquePtr<AssetEntry>, AssetEntryExtractKey> g_assets;
        HashMap<Path, asset_t> g_asset_path_mapping;
        void init_asset_registry()
        {
            register_struct_type<AssetMetaFile>({
                luproperty(AssetMetaFile, Guid, guid),
                luproperty(AssetMetaFile, Name, type)
                });
            set_serializable<AssetMetaFile>();
            g_assets_mutex = new_mutex();
        }
        void close_asset_registry()
        {
            g_assets.clear();
            g_assets.shrink_to_fit();
            g_asset_path_mapping.clear();
            g_asset_path_mapping.shrink_to_fit();
        }
        inline AssetState internal_get_asset_state(AssetEntry* entry)
        {
            if (entry->type.empty()) return AssetState::unregistered;
            if (entry->data) return AssetState::loaded;
            if (entry->loading) return AssetState::loading;
            return AssetState::unloaded;
        }
        static R<AssetMetaFile> internal_load_asset_meta(const Path& meta_path)
        {
            // Read file.
            AssetMetaFile file;
            lutry
            {
                lulet(f, VFS::open_file(meta_path, FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
                lulet(var, VariantUtils::read_json(f));
                luexp(deserialize(file, var));
            }
            lucatch
            {
                if (luerr == BasicError::not_found())
                {
                    return set_error(AssetError::meta_file_not_found(), "Asset meta file %s is not found.", meta_path.encode().c_str());
                }
                return luerr;
            }
            return file;
        }
        static RV internal_save_asset_meta(const AssetMetaFile& file, const Path& meta_path)
        {
            lutry
            {
                lulet(f, VFS::open_file(meta_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
                lulet(data, serialize(file));
                luexp(VariantUtils::write_json(f, data));
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API asset_t get_asset(const Guid& guid)
        {
            MutexGuard g(g_assets_mutex);
            if (guid != Guid(0, 0))
            {
                auto iter = g_assets.find(guid);
                if (iter != g_assets.end())
                {
                    asset_t ret;
                    ret.handle = iter->get();
                    return ret;
                }
            }
            UniquePtr<AssetEntry> entry(memnew<AssetEntry>());
            entry->guid = guid == Guid(0, 0) ? random_guid() : guid;
            asset_t ret;
            ret.handle = entry.get();
            g_assets.insert(move(entry));
            return ret;
        }
        LUNA_ASSET_API RV register_asset(asset_t asset, const Name& type)
        {
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard lock(entry->lock);
            if(internal_get_asset_state(entry) != AssetState::unregistered) return AssetError::asset_already_registered();
            entry->type = type;
            return ok;
        }
        LUNA_ASSET_API R<asset_t> new_asset(const Path& path, const Name& type, bool save_meta_to_file)
        {
            if(!path.empty())
            {
                MutexGuard g1(g_assets_mutex);
                auto iter = g_asset_path_mapping.find(path);
                if (iter != g_asset_path_mapping.end()) return iter->second;
            }
            asset_t ret = get_asset();
            if(save_meta_to_file)
            {
                if(path.empty()) return AssetError::empty_asset_path();
                AssetMetaFile file;
                file.type = type;
                file.guid = get_asset_guid(ret);
                Path meta_path = path;
                meta_path.append_extension("meta");
                auto r = internal_save_asset_meta(file, meta_path);
                if (failed(r)) return r.errcode();
            }
            AssetEntry* entry = (AssetEntry*)ret.handle;
            LockGuard lock(entry->lock);
            entry->path = path;
            entry->type = type;
            if(!path.empty())
            {
                g_asset_path_mapping.insert(make_pair(path, ret));
            }
            return ret;
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
            lutry
            {
                // Collect assets to be updated.
                Vector<AssetMetaUpdateInfo> update_assets;
                auto attr = VFS::get_file_attribute(path);
                if(succeeded(attr) && test_flags(attr.get().attributes, FileAttributeFlag::directory))
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
                // Do update.
                MutexGuard g(g_assets_mutex);
                for(auto& info : update_assets)
                {
                    auto asset = get_asset(info.meta_file.guid);
                    AssetEntry* entry = (AssetEntry*)asset.handle;
                    LockGuard g2(entry->lock);
                    auto state = internal_get_asset_state(entry);
                    if(state == AssetState::unregistered || allow_overwrite)
                    {
                        if(state != AssetState::unregistered)
                        {
                            auto iter = g_asset_path_mapping.find(entry->path);
                            if(iter != g_asset_path_mapping.end() && iter->second == asset)
                            {
                                g_asset_path_mapping.erase(iter);    
                            }
                        }
                        entry->type = info.meta_file.type;
                        entry->path = info.path;
                        g_asset_path_mapping.insert(make_pair(entry->path, asset));
                    }
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API RV load_asset_meta(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            lutry
            {
                Path meta_path = get_asset_path(asset);
                if(meta_path.empty()) return set_error(BasicError::bad_arguments(), "Asset::load_asset_meta: Asset path is not valid");
                meta_path.append_extension("meta");
                lulet(meta_file, internal_load_asset_meta(meta_path));
                AssetEntry* entry = (AssetEntry*)asset.handle;
                LockGuard lock(entry->lock);
                if(entry->guid != meta_file.guid)
                {
                    return set_error(BasicError::bad_data(), "Asset::load_asset_meta: The asset GUID loaded from metadata file does not match the asset GUID in system.");
                }
                entry->type = meta_file.type;
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API RV save_asset_meta(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            lutry
            {
                Path meta_path = get_asset_path(asset);
                if(meta_path.empty()) return set_error(BasicError::bad_arguments(), "Asset::load_asset_meta: Asset path is not valid");
                meta_path.append_extension("meta");
                AssetEntry* entry = (AssetEntry*)asset.handle;
                LockGuard lock(entry->lock);
                AssetMetaFile meta_file;
                meta_file.type = entry->type;
                meta_file.guid = entry->guid;
                lock.unlock();
                luexp(internal_save_asset_meta(meta_file, meta_path));
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API R<asset_t> get_asset_by_path(const Path& path)
        {
            MutexGuard g1(g_assets_mutex);
            auto iter = g_asset_path_mapping.find(path);
            if (iter != g_asset_path_mapping.end()) return iter->second;
            return BasicError::not_found();
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
            MutexGuard g1(g_assets_mutex);
            LockGuard guard(entry->lock);
            auto iter = g_asset_path_mapping.insert(make_pair(path, asset));
            if(!iter.second) return BasicError::already_exists();
            g_asset_path_mapping.erase(entry->path);
            entry->path = path;
            return ok;
        }
        LUNA_ASSET_API Name get_asset_name(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if (entry->path.empty()) return Name();
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
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            entry->type = type;
        }
        LUNA_ASSET_API RV get_asset_files(asset_t asset, Vector<Name>& filenames)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard guard(entry->lock);
            if (entry->path.empty()) return AssetError::asset_not_registered();
            Path path = entry->path;
            guard.unlock();
            Name filename = path.back();
            // Gets the directory.
            path.pop_back();
            lutry
            {
                lulet(iter, VFS::open_dir(path));
                while (iter->is_valid())
                {
                    if (!test_flags(iter->get_attributes(), FileAttributeFlag::directory))
                    {
                        auto name = iter->get_filename();
                        auto name_size = strlen(name);
                        if (!memcmp(name, filename.c_str(), filename.size()))
                        {
                            // match exactly name or extension.
                            if (name_size == filename.size() ||
                                (name_size > (filename.size() + 1) && name[filename.size()] == '.'))
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
            lutry
            {
                Vector<Name> files;
                luexp(get_asset_files(asset, files));
                auto path = get_asset_path(asset);
                {
                    MutexGuard g(g_assets_mutex);
                    g_asset_path_mapping.erase(path);
                }
                path.pop_back();
                for (auto& f : files)
                {
                    path.push_back(f);
                    luexp(VFS::delete_file(path));
                    path.pop_back();
                }
                AssetEntry* entry = (AssetEntry*)asset.handle;
                LockGuard guard(entry->lock);
                entry->reset();
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API RV move_asset(asset_t asset, const Path& new_path)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            MutexGuard g1(g_assets_mutex);
            {
                auto iter = g_asset_path_mapping.find(new_path);
                if (iter != g_asset_path_mapping.end()) return BasicError::already_exists();
            }
            lutry
            {
                auto from_path = get_asset_path(asset);
                if (from_path == new_path) return ok;
                auto to_path = new_path;
                Vector<Name> files;
                luexp(get_asset_files(asset, files));
                Name old_filename = from_path.back();
                Name new_file_name = to_path.back();
                from_path.pop_back();
                to_path.pop_back();
                for (auto& f : files)
                {
                    from_path.push_back(f);
                    to_path.push_back(new_file_name);
                    const usize extension_begin = old_filename.size() + 1;
                    if (f.size() > extension_begin)
                    {
                        to_path.append_extension(f.c_str() + extension_begin);
                    }
                    luexp(VFS::move_file(from_path, to_path, FileMoveFlag::fail_if_exists));
                    from_path.pop_back();
                    to_path.pop_back();
                }
                AssetEntry* entry = (AssetEntry*)asset.handle;
                LockGuard guard(entry->lock);
                g_asset_path_mapping.erase(entry->path);
                entry->path = new_path;
                g_asset_path_mapping.insert(make_pair(entry->path, asset));
                AssetMetaFile meta_file;
                meta_file.type = entry->type;
                meta_file.guid = entry->guid;
                guard.unlock();
                Path meta_path = new_path;
                meta_path.append_extension("meta");
                luexp(internal_save_asset_meta(meta_file, meta_path));
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API ObjRef get_asset_data(asset_t asset)
        {
            if(!asset.handle) return ObjRef();
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard g(entry->lock);
            return entry->data;
        }
        LUNA_ASSET_API RV set_asset_data(asset_t asset, object_t data)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard g(entry->lock);
            lutry
            {
                lulet(desc, get_asset_type_desc(entry->type));
                if (desc.on_set_asset_data)
                {
                    luexp(desc.on_set_asset_data(desc.userdata.get(), asset, data));
                }
            }
            lucatchret;
            entry->data = data;
            return ok;
        }
        LUNA_ASSET_API RV load_asset(asset_t asset, bool force_reload)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard g(entry->lock);
            if((entry->data.valid() || entry->loading) && !force_reload) return ok;
            if(entry->loading) return AssetError::asset_data_loading();
            // Load asset data from this thread.
            Path path = entry->path;
            Name type = entry->type;
            entry->loading = true;
            g.unlock();
            lutry
            {
                if (type.empty())
                {
                    luthrow(AssetError::asset_not_registered());
                }
                if (path.empty())
                {
                    luthrow(AssetError::empty_asset_path());
                }
                lulet(desc, get_asset_type_desc(type));
                ObjRef data;
                if (desc.on_load_asset)
                {
                    luset(data, desc.on_load_asset(desc.userdata.get(), asset, path));
                }
                else
                {
                    luthrow(set_error(BasicError::not_supported(), "Asset loading is not implemented by asset %s", type.c_str()));
                }
                g = entry->lock;
                entry->data = data;
                entry->loading = false;
                g.unlock();
            }
            lucatch
            {
                g = entry->lock;
                entry->loading = false;
                g.unlock();
                return luerr;
            }
            return ok;
        }
        LUNA_ASSET_API RV load_asset_default_data(asset_t asset, bool force_reload)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard g(entry->lock);
            if((entry->data.valid() || entry->loading) && !force_reload) return ok;
            if(entry->loading) return AssetError::asset_data_loading();
            // Load asset data from this thread.
            Name type = entry->type;
            entry->loading = true;
            g.unlock();
            lutry
            {
                if (type.empty())
                {
                    luthrow(AssetError::asset_not_registered());
                }
                lulet(desc, get_asset_type_desc(entry->type));
                ObjRef data;
                if (desc.on_load_asset_default_data)
                {
                    luset(data, desc.on_load_asset_default_data(desc.userdata.get(), asset));
                }
                else
                {
                    luthrow(set_error(BasicError::not_supported(), "Asset default data loading is not implemented by asset %s", entry->type.c_str()));
                }
                g = entry->lock;
                entry->data = data;
                entry->loading = false;
                g.unlock();
            }
            lucatch
            {
                g = entry->lock;
                entry->loading = false;
                g.unlock();
                return luerr;
            }
            return ok;
        }
        LUNA_ASSET_API AssetState get_asset_state(asset_t asset)
        {
            if(!asset.handle) return AssetState::unregistered;
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard g(entry->lock);
            return internal_get_asset_state(entry);
        }
        LUNA_ASSET_API RV save_asset(asset_t asset)
        {
            lucheck_msg(asset.handle, "Asset handle must not be null!");
            AssetEntry* entry = (AssetEntry*)asset.handle;
            LockGuard g(entry->lock);
            if (entry->type.empty() || entry->path.empty()) return AssetError::asset_not_registered();
            if (!entry->data.valid()) return AssetError::asset_data_not_loaded();
            // Save asset.
            lutry
            {
                lulet(desc, get_asset_type_desc(entry->type));
                ObjRef data = entry->data;
                Path path = entry->path;
                g.unlock();
                if (desc.on_save_asset)
                {
                    luexp(desc.on_save_asset(desc.userdata.get(), asset, path, data.get()));
                }
                else
                {
                    luthrow(set_error(BasicError::not_supported(), "Asset Saving is not Implemented by Asset %s", entry->type.c_str()));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_ASSET_API void close()
        {
            MutexGuard guard(g_assets_mutex);
            MutexGuard guard2(g_asset_types_mutex);
            close_asset_registry();
            close_asset_type();
        }
        struct AssetModule : public Module
        {
            virtual const c8* get_name() override { return "Asset"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_variant_utils(), module_vfs()});
            }
            virtual RV on_init() override
            {
                init_asset_type();
                init_asset_registry();
                register_struct_type<asset_t>({});
                SerializableTypeDesc desc;
                desc.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
                {
                    const asset_t* obj = (const asset_t*)inst;
                    if (!obj->handle) return Variant();
                    auto guid = Asset::get_asset_guid(*obj);
                    return serialize(guid);
                };
                desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data) -> RV
                {
                    lutry
                    {
                        asset_t* obj = (asset_t*)inst;
                        if (data.empty()) obj->handle = nullptr;
                        Guid guid = Guid(0, 0);
                        luexp(deserialize(guid, data));
                        *obj = Asset::get_asset(guid);
                    }
                    lucatchret;
                    return ok;
                };
                set_serializable<asset_t>(&desc);
                return ok;
            }
            virtual void on_close() override
            {
                close_asset_registry();
                close_asset_type();
                g_assets_mutex.reset();
                g_asset_types_mutex.reset();
            }
        };
    }

    LUNA_ASSET_API Module* module_asset()
    {
        static Asset::AssetModule m;
        return &m;
    }

    namespace AssetError
    {
        LUNA_ASSET_API errcat_t errtype()
        {
            static errcat_t v = get_error_category_by_name("AssetError");
            return v;
        }
        LUNA_ASSET_API ErrCode meta_file_not_found()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "meta_file_not_found");
            return v;
        }
        LUNA_ASSET_API ErrCode unknown_asset_type()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "unknown_asset_type");
            return v;
        }
        LUNA_ASSET_API ErrCode asset_not_registered()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "asset_not_registered");
            return v;
        }
        LUNA_ASSET_API ErrCode asset_already_registered()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "asset_already_registered");
            return v;
        }
        LUNA_ASSET_API ErrCode empty_asset_path()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "empty_asset_path");
            return v;
        }
        LUNA_ASSET_API ErrCode asset_data_not_loaded()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "asset_data_not_loaded");
            return v;
        }
        LUNA_ASSET_API ErrCode asset_data_loading()
        {
            static ErrCode v = get_error_code_by_name("AssetError", "asset_data_loading");
            return v;
        }
    }
}
