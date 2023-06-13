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
		struct DriverDesc
		{
			void* driver_data;
			void (*driver_close)(void* driver_data);
			R<void*>(*mount)(void* driver_data, const c8* driver_path, const Path& mount_dir, typeinfo_t params_type, void* params_data);
			RV(*unmount)(void* driver_data, void* mount_data);
			R<Ref<IFile>>(*open_file)(void* driver_data, void* mount_data, const Path& path, FileOpenFlag flags, FileCreationMode creation);
			R<FileAttribute>(*get_file_attribute)(void* driver_data, void* mount_data, const Path& path);
			RV(*copy_file)(void* driver_data, void* from_mount_data, void* to_mount_data, const Path& from_path, const Path& to_path, FileCopyFlag flags);
			RV(*move_file)(void* driver_data, void* from_mount_data, void* to_mount_data, const Path& from_path, const Path& to_path, FileMoveFlag flags);
			RV(*delete_file)(void* driver_data, void* mount_data, const Path& path);
			R<Ref<IFileIterator>>(*open_dir)(void* driver_data, void* mount_data, const Path& path);
			RV(*create_dir)(void* driver_data, void* mount_data, const Path& path);
			R<Name>(*get_native_path)(void* driver_data, void* mount_data, const Path& path);
		};

		LUNA_VFS_API void register_driver(const Name& name, const DriverDesc& desc);
	}
}