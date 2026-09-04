/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DocumentFileSystem.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            struct DocumentDirectoryMount
            {
                Path native_directory;
                Path mount_path;
            };

            // Keeps file-dialog selections addressable by Asset without replacing the workspace mount.
            struct DocumentFileSystem
            {
                Vector<DocumentDirectoryMount> external_mounts;
                u64 next_mount_id = 0;

                R<Path> resolve_document_path(const Path& workspace_root, Path native_path);
                RV close();
            };

            // Registers an existing sidecar before Open/Save As, without reloading live assets.
            RV load_document_meta(const Path& asset_path, bool allow_missing);
        }
    }
}
