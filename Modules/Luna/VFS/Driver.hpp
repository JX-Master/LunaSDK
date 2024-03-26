/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Driver.hpp
* @author JXMaster
* @date 2022/5/24
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
        //! @addtogroup VFS
        //! @{
        
        //! Describes one virtual file system (VFS) driver.
        //! @details One VFS driver describes functions to operate files on a certain kind of media. For example, one ZIP driver may provide 
        //! functions to read files from one ZIP archive, and the file devices created from the driver represents one real ZIP file opened for
        //! reading.
        struct DriverDesc
        {
            //! The user-defined driver data pointer that will be passed to all driver callback functions.
            void* driver_data;
            //! Called when the driver is unregistered from VFS.
            //! @details The user should release all dynamic memory attached to @ref driver_data if any.
            //! @param[in] driver_data The user-provided driver data.
            void (*on_driver_unregister)(void* driver_data);
            //! Called when one new device is mounted.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] driver_path The driver native path passed to @ref mount.
            //! @param[in] mount_dir The directory used as the root directory of the mounted file device.
            //! @param[in] params_type The type of the additional driver parameter object passed to @ref mount.
            //! @param[in] params_data The pointer to the additional driver parameter object passed to @ref mount.
            //! @return Returns the mount data that identifies the mounted device.
            R<void*>(*on_mount)(void* driver_data, const c8* driver_path, const Path& mount_dir, typeinfo_t params_type, void* params_data);
            //! Called when one device is unmounted.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount.
            RV(*on_unmount)(void* driver_data, void* mount_data);
            //! Called when @ref VFS::open_file is called on one file or directory belongs to one device of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount for the device.
            //! @param[in] path The path of the file to open relative to the mount root path.
            //! @param[in] flags The file open flags.
            //! @param[in] creation The file creation flags.
            R<Ref<IFile>>(*on_open_file)(void* driver_data, void* mount_data, const Path& path, FileOpenFlag flags, FileCreationMode creation);
            //! Called when @ref VFS::get_file_attribute is called on one file or directory belongs to one device of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount for the device.
            //! @param[in] path The path of the file or directory to check relative to the mount root path.
            //! @return Returns the file attribute object.
            R<FileAttribute>(*on_get_file_attribute)(void* driver_data, void* mount_data, const Path& path);
            //! Called when @ref VFS::copy_file is called on two files that both belong to devices of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] from_mount_data The mount data returned by @ref on_mount for the device that the file is copied from.
            //! @param[in] to_mount_data The mount data returned by @ref on_mount for the device that the file is copied to.
            //! @param[in] from_path The path of the file to copy from relative to the mount root path.
            //! @param[in] to_path The path of the file to copy to relative to the mount root path.
            //! @param[in] flags The file copy flags.
            RV(*on_copy_file)(void* driver_data, void* from_mount_data, void* to_mount_data, const Path& from_path, const Path& to_path, FileCopyFlag flags);
            //! Called when @ref VFS::move_file is called on two files that both belong to devices of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] from_mount_data The mount data returned by @ref on_mount for the device that the file is moved from.
            //! @param[in] to_mount_data The mount data returned by @ref on_mount for the device that the file is moved to.
            //! @param[in] from_path The path of the file to move from relative to the mount root path.
            //! @param[in] to_path The path of the file to move to relative to the mount root path.
            //! @param[in] flags The file move flags.
            RV(*on_move_file)(void* driver_data, void* from_mount_data, void* to_mount_data, const Path& from_path, const Path& to_path, FileMoveFlag flags);
            //! Called when @ref VFS::delete_file is called on one file that belongs to devices of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount for the device.
            //! @param[in] path The path of the file to delete.
            RV(*on_delete_file)(void* driver_data, void* mount_data, const Path& path);
            //! Called when @ref VFS::open_dir is called on one directory that belongs to devices of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount for the device.
            //! @param[in] path The path of the directory to open.
            //! @return Returns one file iterator used to enumerate files in the opened directory.
            R<Ref<IFileIterator>>(*on_open_dir)(void* driver_data, void* mount_data, const Path& path);
            //! Called when @ref VFS::create_dir is called on one directory that belongs to devices of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount for the device.
            //! @param[in] path The path of the directory to create.
            RV(*on_create_dir)(void* driver_data, void* mount_data, const Path& path);
            //! Called when @ref VFS::get_native_path is called on one path that belongs to devices of this driver.
            //! @param[in] driver_data The user-provided driver data.
            //! @param[in] mount_data The mount data returned by @ref on_mount for the device.
            //! @param[in] path The path to convert.
            //! @return Returns one path string that represents the converted native path.
            R<Name>(*on_get_native_path)(void* driver_data, void* mount_data, const Path& path);
        };

        //! Registers one new VFS driver to the system.
        //! @param[in] name The name of the driver. If one driver that has the same name already exists in the system,
        //! the old driver will be replaced by the new driver.
        //! @param[in] desc The descriptor of the driver.
        LUNA_VFS_API void register_driver(const Name& name, const DriverDesc& desc);

        //! @}
    }
}