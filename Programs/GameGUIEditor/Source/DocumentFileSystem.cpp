/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DocumentFileSystem.cpp
* @author JXMaster
* @date 2026/9/4
*/
#include "DocumentFileSystem.hpp"
#include <Luna/Asset/Asset.hpp>
#include <Luna/VFS/VFS.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            R<Path> DocumentFileSystem::resolve_document_path(const Path& workspace_root,
                Path native_path)
            {
                lutry
                {
                    native_path.normalize();
                    if(!test_flags(native_path.flags(), PathFlag::absolute) || native_path.empty())
                        luthrow(set_error(E_BAD_ARGUMENTS, "Select an absolute document file path."));
                    if(!native_path.extension().empty())
                    {
                        if(native_path.extension() != "json")
                            luthrow(set_error(E_BAD_ARGUMENTS,
                                "GameGUI document files must use the .json extension."));
                        native_path.remove_extension();
                    }
                    if(native_path.is_subpath_of(workspace_root))
                    {
                        Path relative_path;
                        relative_path.assign_relative(workspace_root, native_path);
                        Path asset_path("/");
                        asset_path.append(relative_path);
                        return asset_path;
                    }

                    Path directory = native_path;
                    directory.pop_back();
                    // Match the exact parent directory so opening a parent folder later cannot
                    // give a previously opened file a different Asset registry path.
                    for(const DocumentDirectoryMount& mount : external_mounts)
                    {
                        if(mount.native_directory == directory)
                        {
                            Path asset_path = mount.mount_path;
                            asset_path.push_back(native_path.back());
                            return asset_path;
                        }
                    }
                    lulet(attributes, Luna::get_file_attribute(directory.encode().c_str()));
                    if(!test_flags(attributes.attributes, FileAttributeFlag::directory))
                        luthrow(set_error(E_NOT_DIRECTORY, "The selected parent path is not a directory."));
                    Path mount_path;
                    for(;;)
                    {
                        String name;
                        strprintf(name, "/__GameGUIEditorExternal_%llu", (unsigned long long)++next_mount_id);
                        mount_path = name.c_str();
                        // Do not shadow an existing workspace entry or an existing mount.
                        auto attributes = VFS::get_file_attribute(mount_path);
                        if(attributes.valid()) continue;
                        if(attributes.errcode() != E_NOT_FOUND) luthrow(attributes.errcode());
                        RV mounted = VFS::mount(VFS::get_platform_filesystem_driver(),
                            directory.encode(PathSeparator::system_preferred).c_str(), mount_path);
                        if(mounted.errcode() == E_ALREADY_EXISTS) continue;
                        luexp(mounted);
                        break;
                    }
                    external_mounts.push_back({directory, mount_path});
                    mount_path.push_back(native_path.back());
                    return mount_path;
                }
                lucatchret;
                return E_FAILURE;
            }

            RV DocumentFileSystem::close()
            {
                lutry
                {
                    while(!external_mounts.empty())
                    {
                        luexp(VFS::unmount(external_mounts.back().mount_path));
                        external_mounts.pop_back();
                    }
                }
                lucatchret;
                return ok;
            }

            RV load_document_meta(const Path& asset_path, bool allow_missing)
            {
                auto asset = Asset::get_asset_by_path(asset_path);
                if(asset.valid()) return ok;
                lutry
                {
                    Path meta_path = asset_path;
                    meta_path.append_extension("meta");
                    auto attributes = VFS::get_file_attribute(meta_path);
                    if(!attributes.valid())
                    {
                        if(allow_missing && attributes.errcode() == E_NOT_FOUND) return ok;
                        luthrow(set_error(attributes.errcode(),
                            "Cannot read the GameGUI asset metadata. Keep the .json and .meta files together."));
                    }
                    luexp(Asset::load_assets_meta(asset_path, false));
                    // A copied asset may have the GUID of an asset already loaded elsewhere.
                    // Do not silently move that live asset's registry path or overwrite its files.
                    if(!Asset::get_asset_by_path(asset_path).valid())
                        luthrow(set_error(E_ALREADY_EXISTS,
                            "This asset is already registered at another path. Use Save As to create a separate asset."));
                }
                lucatchret;
                return ok;
            }
        }
    }
}
