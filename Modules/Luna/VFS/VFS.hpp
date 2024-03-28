/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VFS.hpp
* @author JXMaster
* @date 2022/5/24
* @brief The virtual file system API.
*/
#pragma once
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Path.hpp>

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
        //! is actually in. VFS supports user-implemented file system drivers and devices, which allows the file system 
        //! to be extended to support reading files from zip archives, online server, etc.
        //! @{
        
        //! Mounts one virtual file device as one directory in the virtual file system.
        //! @param[in] driver The name of the VFS driver for the virtual file device.
        //! @param[in] driver_path The path passed to the driver, which is usually the native path mapped to the mount path.
        //! @param[in] mount_path The directory used as the root directory of the mounted file device.
        //! @param[in] params_type The type of the additional driver parameter object. See driver docs for details.
        //! @param[in] params_data The pointer to the additional driver parameter object. See driver docs for details.
        LUNA_VFS_API RV mount(const Name& driver, const c8* driver_path, const Path& mount_path, 
            typeinfo_t params_type = nullptr, void* params_data = nullptr);

        //! Unmounts the virtual file device in the mounting directory.
        //! @param[in] mount_path The mounting directory specified when mounting or remounting the file device.
        LUNA_VFS_API RV unmount(const Path& mount_path);

        //! Changes the mounting directory of the file device.
        //! @param[in] from_path The current mounting directory of the device.
        //! @param[in] to_path The new mounting directory to change to.
        LUNA_VFS_API RV remount(const Path& from_path, const Path& to_path);

        //! Opens one file.
        //! @param[in] path The path of the file.
        //! @param[in] flags The file open flags.
        //! @param[in] creation Specify whether to create a file if the file does not exist.
        //! @return Returns the new opened file object.
        //! @par Possible Errors:
        //! * BasicError::bad_arguments
        //! * BasicError::access_denied
        //! * BasicError::not_found
        //! * BasicError::not_directory
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API R<Ref<IFile>> open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation);
        //! Gets the file or directory attribute.
        //! @param[in] path The path of the file to check.
        //! @return Returns the file attribute structure.
        //! @par Possible Errors:
        //! * BasicError::access_denied
        //! * BasicError::not_found
        //! * BasicError::not_directory
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API R<FileAttribute> get_file_attribute(const Path& path);
        //! Copies the file or directory from the source path to the destination path.
        //! @param[in] from_path Source file or directory path.
        //! @param[in] to_path Destination file or directory path.
        //! @param[in] flags Additional flags.
        //! @par Possible Errors:
        //! * BasicError::bad_arguments
        //! * BasicError::already_exists
        //! * BasicError::access_denied
        //! * BasicError::not_found
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API RV copy_file(const Path& from_path, const Path& to_path, FileCopyFlag flags = FileCopyFlag::none);
        //! Moves the file or directory from the source path to the destination path. This call can also be used to rename a file.
        //! @param[in] from_path Source file path.
        //! @param[in] to_path Destination file path.
        //! @par Possible Errors:
        //! * BasicError::bad_arguments
        //! * BasicError::already_exists
        //! * BasicError::access_denied
        //! * BasicError::not_found
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API RV move_file(const Path& from_path, const Path& to_path, FileMoveFlag flags = FileMoveFlag::none);
        //! Deletes the specified file.
        //! @param[in] path The file to delete.
        //! @par Possible Errors:
        //! * BasicError::bad_arguments
        //! * BasicError::not_found
        //! * BasicError::access_denied
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API RV delete_file(const Path& path);
        //! Creates a file iterator that can be used to iterate all files in the specified directory.
        //! @param[in] path The directory path to open.
        //! @return Returns a file iterator object.
        //! @par Possible Errors:
        //! * BasicError::not_found
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API R<Ref<IFileIterator>> open_dir(const Path& path);
        //! Creates one directory.
        //! @param[in] path The path of the directory.
        //! @par Possible Errors:
        //! * BasicError::already_exists
        //! * BasicError::not_found
        //! * BasicError::bad_platform_call for all errors that cannot be identified.
        LUNA_VFS_API RV create_dir(const Path& path);

        //! Translates one VFS path to one native driver path.
        //! @param[in] vfs_path The virtual file system path to translate.
        //! @return Returns the translated native path of the virtual path. The translated path is driver-specific.
        LUNA_VFS_API R<Name> get_native_path(const Path& vfs_path);

        //! Gets the name of the VFS driver that maps platform's native file system to vritual file system.
        //! @return Returns the name of the platform's native file system driver.
        LUNA_VFS_API Name get_platform_filesystem_driver();

        //! @}
    }

    //! @addtogroup VFS
    //! @{
    //! @defgroup VFSError VFS Errors
    //! @}

    namespace VFSError
    {
        //! @addtogroup VFSError
        //! @{
        
        LUNA_VFS_API errcat_t errtype();

        //! The specified VFS driver is not found.
        LUNA_VFS_API ErrCode driver_not_found();

        //! @}
    }

    struct Module;
    LUNA_VFS_API Module* module_vfs();
}