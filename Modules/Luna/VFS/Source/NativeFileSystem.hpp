/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file NativeFileSystem.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include "../NativeFileSystem.hpp"
#include "NativeFileSystem.generated.hpp"

namespace Luna::VFS
{
    struct [[Luna::struct("{9FD7D494-E45D-401C-A8E9-DB67C6484DAC}")]] NativeFileSystem : IFileSystem
    {
        luiimpl();
        Path m_root;
        R<String> resolve(const Path& path);
        R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation) override;
        R<FileAttribute> get_file_attribute(const Path& path) override;
        R<Ref<IFileIterator>> open_dir(const Path& path) override;
        RV copy_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system) override;
        RV move_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system) override;
        RV delete_file(const Path& path) override;
        RV create_dir(const Path& path) override;
        R<Name> get_native_path(const Path& path) override;
    };
}
