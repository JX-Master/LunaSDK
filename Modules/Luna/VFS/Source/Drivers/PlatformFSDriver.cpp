/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PlatformFSDriver.cpp
* @author JXMaster
* @date 2022/5/24
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VFS_API LUNA_EXPORT
#include "PlatformFSDriver.hpp"
#include "../../VFS.hpp"
#include "../../Driver.hpp"
namespace Luna
{
	namespace VFS
	{
		struct PlatformFileSystemMountData
		{
			Path native_path;

			Path make_native_path(const Path& path)
			{
				Path ret = native_path;
				ret.append(path);
				return ret;
			}
			String make_native_path_str(const Path& path)
			{
				auto ret_path = make_native_path(path);
				auto ret = ret_path.encode();
				return ret;
			}
		};

		static R<void*> fs_mount(void* driver_data, const c8* driver_path, const Path& mount_dir, typeinfo_t params_type, void* params_data)
		{
			PlatformFileSystemMountData* data = memnew<PlatformFileSystemMountData>();
			data->native_path = driver_path;
			return data;
		}
		static RV fs_unmount(void* driver_data, void* mount_data)
		{
			memdelete((PlatformFileSystemMountData*)mount_data);
			return ok;
		}
		static R<Ref<IFile>> fs_open_file(void* driver_data, void* mount_data, const Path& path, FileOpenFlag flags, FileCreationMode creation)
		{
			auto data = (PlatformFileSystemMountData*)mount_data;
			auto native_path = data->make_native_path_str(path);
			return Luna::open_file(native_path.c_str(), flags, creation);
		}
		static R<FileAttribute> fs_get_file_attribute(void* driver_data, void* mount_data, const Path& path)
		{
			auto data = (PlatformFileSystemMountData*)mount_data;
			auto native_path = data->make_native_path_str(path);
			return Luna::get_file_attribute(native_path.c_str());
		}
		static RV fs_copy_file(void* driver_data, void* from_mount_data, void* to_mount_data, const Path& from_path, const Path& to_path, FileCopyFlag flags)
		{
			auto from_data = (PlatformFileSystemMountData*)from_mount_data;
			auto to_data = (PlatformFileSystemMountData*)to_mount_data;
			auto from = from_data->make_native_path_str(from_path);
			auto to = to_data->make_native_path_str(to_path);
			return Luna::copy_file(from.c_str(), to.c_str(), flags);
		}
		static RV fs_move_file(void* driver_data, void* from_mount_data, void* to_mount_data, const Path& from_path, const Path& to_path, FileMoveFlag flags)
		{
			auto from_data = (PlatformFileSystemMountData*)from_mount_data;
			auto to_data = (PlatformFileSystemMountData*)to_mount_data;
			auto from = from_data->make_native_path_str(from_path);
			auto to = to_data->make_native_path_str(to_path);
			return Luna::move_file(from.c_str(), to.c_str(), flags);
		}
		static RV fs_delete_file(void* driver_data, void* mount_data, const Path& path)
		{
			auto data = (PlatformFileSystemMountData*)mount_data;
			auto native_path = data->make_native_path_str(path);
			return Luna::delete_file(native_path.c_str());
		}
		static R<Ref<IFileIterator>> fs_open_dir(void* driver_data, void* mount_data, const Path& path)
		{
			auto data = (PlatformFileSystemMountData*)mount_data;
			auto native_path = data->make_native_path_str(path);
			return Luna::open_dir(native_path.c_str());
		}
		static RV fs_create_dir(void* driver_data, void* mount_data, const Path& path)
		{
			auto data = (PlatformFileSystemMountData*)mount_data;
			auto native_path = data->make_native_path_str(path);
			return Luna::create_dir(native_path.c_str());
		}
		static R<Name> fs_get_native_path(void* driver_data, void* mount_data, const Path& path)
		{
			auto data = (PlatformFileSystemMountData*)mount_data;
			auto native_path = data->make_native_path_str(path);
			return Name(native_path);
		}
		void register_platform_filesystem_driver()
		{
			DriverDesc desc;
			desc.driver_data = nullptr;
			desc.driver_close = nullptr;
			desc.mount = fs_mount;
			desc.unmount = fs_unmount;
			desc.open_file = fs_open_file;
			desc.get_file_attribute = fs_get_file_attribute;
			desc.copy_file = fs_copy_file;
			desc.move_file = fs_move_file;
			desc.delete_file = fs_delete_file;
			desc.open_dir = fs_open_dir;
			desc.create_dir = fs_create_dir;
			desc.get_native_path = fs_get_native_path;
			register_driver(get_platform_filesystem_driver(), desc);
		}
		LUNA_VFS_API Name get_platform_filesystem_driver()
		{
			return "Platform File System";
		}
	}
}