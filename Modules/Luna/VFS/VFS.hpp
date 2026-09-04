/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VFS.hpp
* @author JXMaster
* @date 2022/5/24
* @brief The virtual file system API.
*/
#pragma once
#include "FileSystem.hpp"

#ifndef LUNA_VFS_API
#define LUNA_VFS_API
#endif

namespace Luna
{
    namespace VFS
    {
        //! @addtogroup VFS VFS
        //! Virtual file system (VFS) module provides a virtual and uniform file system across all platforms, so that
        //! the user can use one uniform path and API to open one file on all platforms without caring where the file 
        //! is actually in. VFS mounts independently created IFileSystem instances to
        //! support native directories, packages and application-defined storage.
        //! @{
        
        //! Mounts an existing filesystem instance as a directory in VFS.
        //! @param[in] file_system The filesystem instance to retain. Must not be null.
        //! @param[in] mount_path The directory used as the root directory of the mounted file device.
        //! @details No storage is created or opened here. One instance may have multiple
        //! mount aliases. Routing uses the longest matching prefix in the same root and
        //! absolute/relative namespace. Filesystem methods invoked by VFS must not reenter VFS.
        LUNA_VFS_API RV mount(IFileSystem* file_system, const Path& mount_path);

        //! Unmounts the virtual file device in the mounting directory.
        //! @param[in] mount_path The mounting directory specified when mounting or remounting the file device.
        //! @details Requires all files and directory iterators of this mount to be released.
        //! Calls IFileSystem::flush before removing the mount; errors retain it for retry.
        //! The filesystem remains usable through other mounts or caller-held references.
        LUNA_VFS_API RV unmount(const Path& mount_path);

        //! Calls IFileSystem::flush once for each distinct mounted instance and returns
        //! the first failure with its diagnostic preserved. Later instances are still
        //! attempted after a failure. Flush a specific instance directly through its interface.
        LUNA_VFS_API RV flush_all();

        //! Changes the mounting directory of the file device.
        //! @param[in] from_path The current mounting directory of the device.
        //! @param[in] to_path The new mounting directory to change to.
        //! @details An occupied destination returns E_ALREADY_EXISTS.
        LUNA_VFS_API RV remount(const Path& from_path, const Path& to_path);

        //! Opens one file.
        //! @param[in] path The path of the file.
        //! @param[in] flags The file open flags.
        //! @param[in] creation Specify whether to create a file if the file does not exist.
        //! @return Returns the new opened file object.
        //! @par Possible Errors:
        //! * E_BAD_ARGUMENTS
        //! * E_ACCESS_DENIED
        //! * E_NOT_FOUND
        //! * E_NOT_DIRECTORY
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation);
        //! Gets the file or directory attribute.
        //! @param[in] path The path of the file to check.
        //! @return Returns the file attribute structure.
        //! @par Possible Errors:
        //! * E_ACCESS_DENIED
        //! * E_NOT_FOUND
        //! * E_NOT_DIRECTORY
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API R<FileAttribute> get_file_attribute(const Path& path);
        //! Copies the file from the source path to the destination path.
        //! This function cannot copy directories.
        //! @param[in] from_path Source file path.
        //! @param[in] to_path Destination file path.
        //! @par Possible Errors:
        //! * E_BAD_ARGUMENTS
        //! * E_ALREADY_EXISTS
        //! * E_ACCESS_DENIED
        //! * E_NOT_FOUND
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API RV copy_file(const Path& from_path, const Path& to_path);
        //! Moves the file or directory from the source path to the destination path. This call can also be used to rename a file.
        //! @param[in] from_path Source file path.
        //! @param[in] to_path Destination file path.
        //! @par Possible Errors:
        //! * E_BAD_ARGUMENTS
        //! * E_ALREADY_EXISTS
        //! * E_ACCESS_DENIED
        //! * E_NOT_FOUND
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API RV move_file(const Path& from_path, const Path& to_path);
        //! Deletes the specified file or directory
        //! @param[in] path The file to delete.
        //! If this specifies one directory, the directory must be empty.
        //! @par Possible Errors:
        //! * E_BAD_ARGUMENTS
        //! * E_NOT_FOUND
        //! * E_ACCESS_DENIED
        //! * E_DIRECTORY_NOT_EMPTY
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API RV delete_file(const Path& path);
        //! Creates a file iterator that can be used to iterate all files in the specified directory.
        //! @param[in] path The directory path to open.
        //! @return Returns a file iterator object.
        //! @par Possible Errors:
        //! * E_NOT_FOUND
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API R<Ref<IFileIterator>> open_dir(const Path& path);
        //! Creates one directory.
        //! @param[in] path The path of the directory.
        //! @par Possible Errors:
        //! * E_ALREADY_EXISTS
        //! * E_NOT_FOUND
        //! * E_BAD_PLATFORM_CALL for all errors that cannot be identified.
        LUNA_VFS_API RV create_dir(const Path& path);

        //! Translates one VFS path to one native storage path.
        //! @param[in] vfs_path The virtual file system path to translate.
        //! @return Returns the translated native path, or E_NOT_SUPPORTED if the
        //! filesystem entry has no independent native path.
        LUNA_VFS_API R<Name> get_native_path(const Path& vfs_path);

        //! @}
    }

    struct Module;
    LUNA_VFS_API Module* module_vfs();
}
