/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file FileSystem.hpp
* @author JXMaster
* @date 2026/9/4
*/
#pragma once
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Path.hpp>
#include "FileSystem.generated.hpp"

#ifndef LUNA_VFS_API
#define LUNA_VFS_API
#endif

namespace Luna::VFS
{
    //! @addtogroup VFS
    //! @{

    //! An independently usable filesystem instance that can be mounted into VFS.
    //! @details Paths are relative to the addressed instance's root; copy/move
    //! destinations use the destination root. An empty Path names a root.
    //! Absolute paths, named roots and unresolved parent components are
    //! invalid. Implementations own their storage, synchronization and handle lifetimes.
    //! VFS retains mounted instances and may mount one instance at multiple paths.
    //! Methods invoked by VFS must not reenter VFS. Destruction releases resources;
    //! use checked flush before releasing an instance with deferred writes.
    struct [[Luna::interface("{4371C890-22D1-4C8A-9155-6E62D3B9B88B}")]] IFileSystem : virtual Interface
    {
        //! Opens a file relative to the filesystem root.
        //! @param[in] path The relative file path.
        //! @param[in] flags The requested access flags.
        //! @param[in] creation The file creation policy.
        //! @return Returns a retained file handle. Failure must not create a file.
        virtual R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation) = 0;
        //! Gets the attributes of a file or directory relative to the root.
        //! @param[in] path The relative path, or an empty Path for the root.
        virtual R<FileAttribute> get_file_attribute(const Path& path) = 0;
        //! Enumerates the immediate children of a directory.
        //! @param[in] path The relative directory path, or an empty Path for the root.
        virtual R<Ref<IFileIterator>> open_dir(const Path& path) = 0;
        //! Copies a file without replacing an existing destination.
        //! @param[in] from_path The source file path relative to the root.
        //! @param[in] to_path The destination file path relative to the destination root.
        //! @param[in] to_file_system The destination instance. Null selects this instance.
        //! @details E_NOT_SUPPORTED must leave storage unchanged; VFS then attempts
        //! a generic streaming copy. Directory copying is not supported.
        virtual RV copy_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system = nullptr) { return E_NOT_SUPPORTED; }
        //! Moves a file or directory with optional file replacement.
        //! @param[in] from_path The source path relative to the root.
        //! @param[in] to_path The destination path relative to the destination root.
        //! @param[in] to_file_system The destination instance. Null selects this instance.
        //! @param[in] flags The replacement and copy fallback policy. Directories cannot replace existing entries.
        //! @details E_NOT_SUPPORTED must leave storage unchanged. Unless no_copy is set, VFS may
        //! attempt a streaming file copy followed by source deletion into an absent destination.
        virtual RV move_file(const Path& from_path, const Path& to_path, IFileSystem* to_file_system = nullptr,
            FileMoveFlag flags = FileMoveFlag::none) { return E_NOT_SUPPORTED; }
        //! Deletes a file or an empty directory relative to the root.
        //! @param[in] path The relative file or directory path.
        virtual RV delete_file(const Path& path) { return E_NOT_SUPPORTED; }
        //! Creates a directory whose parent already exists.
        //! @param[in] path The relative directory path.
        virtual RV create_dir(const Path& path) { return E_NOT_SUPPORTED; }
        //! Translates a relative path to a native storage path if the instance supports it.
        //! @param[in] path The relative path, or an empty Path for the root.
        //! @return Returns E_NOT_SUPPORTED for entries without independent native paths.
        virtual R<Name> get_native_path(const Path& path) { return E_NOT_SUPPORTED; }
        //! Publishes deferred changes without closing this instance or changing mounts.
        //! @details Implementations retain retryable state on failure. Pak instances
        //! require all of their file handles to be released, including direct handles
        //! and handles from any mount alias. Directory snapshots may remain open.
        //! The default implementation has no deferred work and succeeds.
        virtual RV flush() { return ok; }
    };
    //! @}
}
