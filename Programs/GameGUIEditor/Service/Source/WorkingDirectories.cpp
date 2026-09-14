/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file WorkingDirectories.cpp
* @author JXMaster
* @date 2026/9/11
*/
#include "WorkingDirectories.hpp"
#include "../Authoring.hpp"
#include <Luna/VFS/VFS.hpp>
#include <Luna/VFS/NativeFileSystem.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Atomic.hpp>
#include <filesystem>

namespace Luna::GameGUIEditor
{
    namespace
    {
        // Runtime has no canonical native path API. Use the non-throwing standard API only
        // for path identity; all asset and directory I/O remains in Luna's filesystem layer.
        R<Path> canonical_path(const Path& path)
        {
            std::error_code error;
            auto canonical = std::filesystem::weakly_canonical(
                std::filesystem::path((const char8_t*)path.encode().c_str()), error);
            if(error) return set_error(E_BAD_ARGUMENTS, "Cannot resolve path: %s", error.message().c_str());
            auto utf8 = canonical.generic_u8string();
            return Path((const c8*)utf8.c_str());
        }

        bool contains_path(const Path& root, const Path& path)
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            String root_string = root.encode();
            String path_string = path.encode();
            for(c8& c : root_string) if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
            for(c8& c : path_string) if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
            return Path(path_string.c_str()).is_subpath_of(Path(root_string.c_str()));
#else
            return path.is_subpath_of(root);
#endif
        }

        RV scan_folders(WorkingDirectory& directory, const Path& relative, Vector<Path>& folders)
        {
            lutry
            {
                lulet(iterator, directory.file_system->open_dir(relative));
                while(iterator->is_valid())
                {
                    Name name(iterator->get_filename());
                    if(name != Name(".") && name != Name("..") &&
                        test_flags(iterator->get_attributes(), FileAttributeFlag::directory))
                    {
                        Path child = relative;
                        child.push_back(name);
                        folders.push_back(child);
                        luexp(scan_folders(directory, child, folders));
                    }
                    iterator->move_next();
                }
            }
            lucatchret;
            return ok;
        }
    }

    R<Path> DirectoryFileSystem::check(const Path& path, bool directory)
    {
        if(test_flags(path.flags(), PathFlag::absolute) || !path.root().empty()) return E_BAD_ARGUMENTS;
        for(const Name& part : path) if(part == Name("..")) return E_BAD_ARGUMENTS;
        Path native = native_root;
        native.append(path);
        lutry
        {
            lulet(canonical, canonical_path(native));
            if(!contains_path(native_root, canonical))
                luthrow(set_error(E_BAD_ARGUMENTS, "The path escapes its working directory: %s", native.encode().c_str()));
            // Directory aliases introduce duplicate asset paths or recursive scan cycles.
            if(directory && canonical != native)
                luthrow(set_error(E_NOT_SUPPORTED, "Directory aliases are not supported inside a working directory: %s",
                    native.encode().c_str()));
            return native;
        }
        lucatchret;
        return E_FAILURE;
    }

    R<Ref<IFile>> DirectoryFileSystem::open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation)
    {
        auto checked = check(path);
        if(!checked.valid()) return checked.errcode();
        return storage->open_file(path, flags, creation);
    }
    R<FileAttribute> DirectoryFileSystem::get_file_attribute(const Path& path)
    {
        auto checked = check(path);
        if(!checked.valid()) return checked.errcode();
        return storage->get_file_attribute(path);
    }
    R<Ref<IFileIterator>> DirectoryFileSystem::open_dir(const Path& path)
    {
        auto checked = check(path, true);
        if(!checked.valid()) return checked.errcode();
        return storage->open_dir(path);
    }
    RV DirectoryFileSystem::copy_file(const Path& from, const Path& to, VFS::IFileSystem* destination)
    {
        if(destination && destination != this) return E_NOT_SUPPORTED;
        auto source = check(from);
        if(!source.valid()) return source.errcode();
        auto target = check(to);
        if(!target.valid()) return target.errcode();
        return storage->copy_file(from, to, nullptr);
    }
    RV DirectoryFileSystem::move_file(const Path& from, const Path& to, VFS::IFileSystem* destination, FileMoveFlag flags)
    {
        if(destination && destination != this) return E_NOT_SUPPORTED;
        auto source = check(from);
        if(!source.valid()) return source.errcode();
        auto target = check(to);
        if(!target.valid()) return target.errcode();
        return storage->move_file(from, to, nullptr, flags);
    }
    RV DirectoryFileSystem::delete_file(const Path& path)
    {
        auto checked = check(path);
        if(!checked.valid()) return checked.errcode();
        return storage->delete_file(path);
    }
    RV DirectoryFileSystem::create_dir(const Path& path)
    {
        auto checked = check(path);
        if(!checked.valid()) return checked.errcode();
        return storage->create_dir(path);
    }
    R<Name> DirectoryFileSystem::get_native_path(const Path& path)
    {
        auto checked = check(path);
        if(!checked.valid()) return checked.errcode();
        return Name(checked.get().encode());
    }
    RV DirectoryFileSystem::flush() { return storage->flush(); }

    R<WorkingDirectory*> WorkingDirectories::get(u64 id, bool allow_closing)
    {
        for(auto& directory : directories)
        {
            if(directory->id != id) continue;
            if(directory->closing && !allow_closing) return set_error(E_BUSY, "The working directory is closing.");
            return directory.get();
        }
        return set_error(E_NOT_FOUND, "Open a working directory first.");
    }

    R<WorkingDirectory*> WorkingDirectories::owner(const Path& path, bool allow_closing)
    {
        for(auto& directory : directories)
        {
            if(!path.is_subpath_of(directory->vfs_root)) continue;
            return get(directory->id, allow_closing);
        }
        return set_error(E_NOT_FOUND, "The asset is outside the opened working directories. Open its directory first.");
    }

    R<WorkingDirectory*> WorkingDirectories::open(const Path& native_root)
    {
        UniquePtr<WorkingDirectory> directory(memnew<WorkingDirectory>());
        lutry
        {
            luset(directory->native_root, canonical_path(native_root));
            lulet(attributes, Luna::get_file_attribute(directory->native_root.encode().c_str()));
            if(!test_flags(attributes.attributes, FileAttributeFlag::directory)) luthrow(E_NOT_DIRECTORY);
            for(auto& existing : directories)
            {
                std::error_code error;
                bool same = std::filesystem::equivalent(std::filesystem::path((const char8_t*)existing->native_root.encode().c_str()),
                    std::filesystem::path((const char8_t*)directory->native_root.encode().c_str()), error);
                if(!error && same) return get(existing->id);
                if(contains_path(existing->native_root, directory->native_root) ||
                    contains_path(directory->native_root, existing->native_root))
                    luthrow(set_error(E_ALREADY_EXISTS, "Working directories must not contain each other."));
            }
            // Process-wide IDs avoid collisions between independent headless services.
            static volatile u64 next_id = 0;
            directory->id = atom_inc_u64(&next_id);
            String mount;
            strprintf(mount, "/GameGUIEditor/WorkingDirectories/%llu", (unsigned long long)directory->id);
            directory->vfs_root = mount.c_str();
            directory->file_system = new_object<DirectoryFileSystem>();
            directory->file_system->native_root = directory->native_root;
            luset(directory->file_system->storage, VFS::new_native_file_system(directory->native_root.encode().c_str()));
            luexp(VFS::mount(directory->file_system, directory->vfs_root));
            auto prepare = [&]() -> RV
            {
                lutry
                {
                    auto attributes = directory->file_system->get_file_attribute("assets.db");
                    if(attributes.valid())
                    {
                        luset(directory->database, Asset::open_file_database(directory->vfs_root, "assets.db", Asset::DatabaseMode::read_write));
                    }
                    else if(attributes.errcode() == E_NOT_FOUND)
                    {
                        luset(directory->database, Asset::new_sidecar_database(directory->vfs_root));
                    }
                    else luthrow(attributes.errcode());
                    luexp(Asset::register_asset_database(directory->database));
                    directory->registered = true;
                    luexp(rebuild_index(*directory));
                }
                lucatchret;
                return ok;
            };
            RV prepared = prepare();
            if(failed(prepared))
            {
                ResultCode code = unwrap_errcode(prepared);
                String message = explain(prepared.errcode());
                // Keep cleanup failures reachable for retry rather than losing an owned mount.
                RV cleanup = directory->registered ? Asset::unregister_asset_database(directory->database) : ok;
                if(succeeded(cleanup))
                {
                    directory->registered = false;
                    cleanup = VFS::unmount(directory->vfs_root);
                }
                if(failed(cleanup))
                {
                    message.append(" Directory cleanup is pending; retry Unload Directory.");
                    directory->closing = true;
                    directories.push_back(move(directory));
                }
                return set_error(code, "%s", message.c_str());
            }
        }
        lucatchret;
        WorkingDirectory* result = directory.get();
        directories.push_back(move(directory));
        return result;
    }

    RV WorkingDirectories::rebuild_index(WorkingDirectory& directory)
    {
        lutry
        {
            lulet(records, directory.database->get_records());
            Vector<Path> folders;
            luexp(scan_folders(directory, Path(), folders));
            // Preserve metadata-only folder paths even when payload directories are absent.
            for(const auto& record : records)
            {
                Path parent = record.path;
                parent.pop_back();
                while(!parent.empty())
                {
                    bool found = false;
                    for(const Path& folder : folders) if(folder == parent) { found = true; break; }
                    if(!found) folders.push_back(parent);
                    parent.pop_back();
                }
            }
            directory.records = move(records);
            directory.folders = move(folders);
        }
        lucatchret;
        return ok;
    }

    R<Path> WorkingDirectories::resolve_native(Path path)
    {
        if(!test_flags(path.flags(), PathFlag::absolute)) return E_BAD_ARGUMENTS;
        if(!path.extension().empty() && path.extension() != "json")
            return set_error(E_BAD_ARGUMENTS, "GameGUI authoring files must use the .json extension.");
        lutry
        {
            luset(path, canonical_path(path));
            if(path.extension() == "json") path.remove_extension();
            for(auto& directory : directories)
            {
                if(!contains_path(directory->native_root, path)) continue;
                luexp(get(directory->id, true));
                Path relative;
                relative.assign_relative(directory->native_root, path);
                Path result = directory->vfs_root;
                result.append(relative);
                luexp(validate_asset_path(result, true));
                return result;
            }
        }
        lucatchret;
        return set_error(E_NOT_FOUND, "The selected file is outside the opened working directories. Open its directory first.");
    }

    RV WorkingDirectories::validate_asset_path(const Path& path, bool allow_closing)
    {
        lutry
        {
            lulet(directory, owner(path, allow_closing));
            Path relative;
            relative.assign_relative(directory->vfs_root, path);
            if(relative.empty()) luthrow(E_BAD_ARGUMENTS);
            luexp(directory->file_system->check(relative));
        }
        lucatchret;
        return ok;
    }

    RV WorkingDirectories::flush(WorkingDirectory& directory)
    {
        lutry
        {
            luexp(directory.database->flush());
            luexp(directory.file_system->flush());
        }
        lucatchret;
        return ok;
    }

    RV WorkingDirectories::unload_data(WorkingDirectory& directory)
    {
        lutry
        {
            // Include the registered snapshot and attempted saves even if sidecars were removed
            // or corrupted externally. Unloading must not depend on rereading valid metadata.
            Vector<Asset::asset_t> assets = directory.touched_assets;
            auto collect = [&](const Vector<Asset::AssetMetadata>& records)
            {
                for(const auto& record : records)
                {
                    Asset::asset_t asset = Asset::get_asset(record.guid);
                    bool found = false;
                    for(auto existing : assets) if(existing == asset) { found = true; break; }
                    if(!found) assets.push_back(asset);
                }
            };
            collect(directory.records);
            auto records = directory.database->get_records();
            if(records.valid()) collect(records.get());
            for(Asset::asset_t asset : assets)
            {
                if(!Asset::get_asset_path(asset).is_subpath_of(directory.vfs_root)) continue;
                Vector<Asset::AssetDataUnitDesc> units;
                auto enumerated = Asset::get_asset_data_units(asset, units);
                if(!enumerated.valid())
                {
                    // Unknown types can have metadata without any installed loader.
                    Asset::AssetDataUnitDesc main;
                    units.push_back(main);
                    for(const auto& record : directory.records)
                        if(record.guid == Asset::get_asset_guid(asset))
                            for(const auto& unit : record.data_units) units.push_back(unit);
                }
                for(const auto& unit : units)
                {
                    auto current = Asset::get_asset_data_unit_state(asset, unit.id);
                    if(!current.valid() && current.errcode() == Asset::E_ASSET_DATA_UNIT_NOT_FOUND) continue;
                    luexp(current);
                    auto state = current.get();
                    if(state == Asset::AssetDataUnitState::loading) luthrow(E_BUSY);
                    if(state == Asset::AssetDataUnitState::loaded)
                        luexp(Asset::set_asset_data_unit_object(asset, unit.id, nullptr));
                }
            }
        }
        lucatchret;
        return ok;
    }

    void WorkingDirectories::track_asset(WorkingDirectory& directory, Asset::asset_t asset)
    {
        bool tracked = false;
        for(auto existing : directory.touched_assets) if(existing == asset) { tracked = true; break; }
        if(!tracked) directory.touched_assets.push_back(asset);
        Asset::AssetMetadata metadata;
        metadata.guid = Asset::get_asset_guid(asset);
        metadata.type = Asset::get_asset_type(asset);
        metadata.path.assign_relative(directory.vfs_root, Asset::get_asset_path(asset));
        Vector<Asset::AssetDataUnitDesc> units;
        if(succeeded(Asset::get_asset_data_units(asset, units)))
            for(const auto& unit : units) if(!unit.id.empty()) metadata.data_units.push_back(unit);
        bool found = false;
        for(auto& record : directory.records)
        {
            if(record.guid != metadata.guid) continue;
            record = metadata;
            found = true;
            break;
        }
        if(!found) directory.records.push_back(metadata);
        Path parent = metadata.path;
        parent.pop_back();
        while(!parent.empty())
        {
            bool exists = false;
            for(const Path& folder : directory.folders) if(folder == parent) { exists = true; break; }
            if(!exists) directory.folders.push_back(parent);
            parent.pop_back();
        }
    }

    RV WorkingDirectories::close(WorkingDirectory& directory)
    {
        lutry
        {
            if(directory.registered)
            {
                luexp(unload_data(directory));
                luexp(Asset::unregister_asset_database(directory.database));
                directory.registered = false;
            }
            RV unmounted = VFS::unmount(directory.vfs_root);
            if(failed(unmounted))
            {
                ResultCode code = unwrap_errcode(unmounted);
                String message = explain(unmounted.errcode());
                // Restore registration when possible; otherwise keep an explicit closing entry.
                RV restored = directory.database ? Asset::register_asset_database(directory.database) : RV(E_BAD_ARGUMENTS);
                directory.registered = succeeded(restored);
                directory.closing = !directory.registered;
                if(!directory.registered) message.append(" Registration could not be restored; retry Unload Directory.");
                return set_error(code, "%s", message.c_str());
            }
            for(usize i = 0; i < directories.size(); ++i)
            {
                if(directories[i].get() == &directory)
                {
                    directories.erase(directories.begin() + i);
                    break;
                }
            }
        }
        lucatchret;
        return ok;
    }

    Variant WorkingDirectories::describe(const WorkingDirectory& directory, bool entries) const
    {
        Variant result(VariantType::object);
        result["working_directory_id"] = directory.id;
        result["native_path"] = directory.native_root.encode().c_str();
        result["vfs_path"] = directory.vfs_root.encode().c_str();
        result["closing"] = directory.closing;
        if(!entries) return result;
        result["folders"] = Variant(VariantType::array);
        for(const Path& folder : directory.folders) result["folders"].push_back(folder.encode().c_str());
        result["assets"] = Variant(VariantType::array);
        for(const auto& record : directory.records)
        {
            Variant asset(VariantType::object);
            c8 guid[GUID_STRING_LENGTH + 1] = {};
            lupanic_if_failed(encode_guid(record.guid, guid, GUID_STRING_LENGTH));
            asset["asset_guid"] = guid;
            asset["type"] = record.type;
            asset["path"] = record.path.encode().c_str();
            bool editable = false;
            for(const auto& unit : record.data_units)
                if(unit.id == get_authoring_data_unit() && unit.loader == get_authoring_asset_loader()) editable = true;
            asset["editable"] = editable;
            result["assets"].push_back(move(asset));
        }
        return result;
    }
}
