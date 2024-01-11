/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VFS.cpp
* @author JXMaster
* @date 2022/5/24
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VFS_API LUNA_EXPORT
#include "VFS.hpp"
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/Module.hpp>
#include "Drivers/PlatformFSDriver.hpp"

namespace Luna
{
	namespace VFS
	{
		HashMap<Name, UniquePtr<DriverDesc>> g_drivers;
		Ref<IMutex> g_driver_mutex;
		Vector<MountPair> g_mounts;
		Ref<IMutex> g_mounts_mutex;
		LUNA_VFS_API void register_driver(const Name& name, const DriverDesc& desc)
		{
			MutexGuard guard(g_driver_mutex);
			auto iter = g_drivers.find(name);
			// If the driver is registered, unregister it first.
			if (iter != g_drivers.end())
			{
				if(iter->second.get()->driver_close) iter->second.get()->driver_close(iter->second.get()->driver_data);
				g_drivers.erase(iter);
			}
			UniquePtr<DriverDesc> d(memnew<DriverDesc>());
			*(d.get()) = desc;
			g_drivers.insert(make_pair(name, move(d)));
		}

		inline DriverDesc* find_driver(const Name& driver)
		{
			MutexGuard guard(g_driver_mutex);
			auto iter = g_drivers.find(driver);
			if (iter == g_drivers.end()) return nullptr;
			return iter->second.get();
		}

		LUNA_VFS_API RV mount(const Name& driver, const c8* driver_path, const Path& mount_path,
			typeinfo_t params_type, void* params_data)
		{
			MutexGuard guard(g_mounts_mutex);
			DriverDesc* d = find_driver(driver);
			if (!d) return VFSError::driver_not_found();
			MountPair p;
			p.m_mount_path = mount_path;
			p.m_driver = d;
			for (auto& i : g_mounts)
			{
				if (p.m_mount_path == i.m_mount_path)
				{
					return BasicError::already_exists();
				}
			}
			lutry
			{
				luset(p.m_mount_data, d->mount(d->driver_data, driver_path, mount_path, params_type, params_data));
			}
			lucatchret;
			g_mounts.push_back(move(p));
			return ok;
		}
		LUNA_VFS_API RV unmount(const Path& mount_path)
		{
			MutexGuard guard(g_mounts_mutex);
			auto iter = g_mounts.begin();
			while (iter != g_mounts.end())
			{
				if (iter->m_mount_path == mount_path)
				{
					lutry
					{
						luexp(iter->m_driver->unmount(iter->m_driver->driver_data, iter->m_mount_data));
					}
					lucatchret;
					g_mounts.erase(iter);
					return ok;
				}
			}
			return BasicError::not_found();
		}
		LUNA_VFS_API RV remount(const Path& from_path, const Path& to_path)
		{
			MutexGuard guard(g_mounts_mutex);
			auto iter = g_mounts.begin();
			while (iter != g_mounts.end())
			{
				if (iter->m_mount_path == from_path)
				{
					iter->m_mount_path = to_path;
					return ok;
				}
			}
			return BasicError::not_found();
		}
		static R<MountPair> route_path(const Path& filename, Path& relative_path)
		{
			auto iter = g_mounts.rbegin();
			while (iter != g_mounts.rend())
			{
				if (filename.is_subpath_of(iter->m_mount_path))
				{
					// matched.
					auto driver = iter->m_driver;
					relative_path = Path();
					relative_path.assign_relative(iter->m_mount_path, filename);
					return *iter;
				}
				else
				{
					++iter;
				}
			}
			// Not found.
			return BasicError::not_found();
		}
		static RV copy_file_between_driver(MountPair* from, MountPair* to, const Path& from_path, const Path& to_path, bool fail_if_exists)
		{
			// try open file.
			Ref<IFile> from_file;
			Ref<IFile> to_file;
			u64 file_sz;
			lutry
			{
				luset(from_file, from->m_driver->open_file(from->m_driver->driver_data, from->m_mount_data, from_path, FileOpenFlag::read, FileCreationMode::open_existing));
				if (fail_if_exists)
				{
					luset(to_file, to->m_driver->open_file(to->m_driver->driver_data, to->m_mount_data, to_path, FileOpenFlag::write, FileCreationMode::create_new));
				}
				else
				{
					luset(to_file, to->m_driver->open_file(to->m_driver->driver_data, to->m_mount_data, to_path, FileOpenFlag::write, FileCreationMode::create_always));
				}
				file_sz = from_file->get_size();
				luexp(to_file->set_size(file_sz));
				// preparing buffer.
				void* alloc_buf = nullptr;
				usize alloc_size = max<usize>(file_sz, 16_mb);
				alloc_buf = memalloc(alloc_size);
				if (!alloc_buf)
				{
					luthrow(BasicError::out_of_memory());
				}
				Blob buf;
				buf.attach((byte_t*)alloc_buf, alloc_size, 0);
				if (file_sz >= 16_mb)
				{
					for (u64 i = 0; i < file_sz; i += 16_mb)
					{
						usize bytes_to_read = (usize)min(16_mb, file_sz - i);
						usize bytes_read;
						luexp(from_file->read(buf.data(), bytes_to_read, &bytes_read));
						luassert(bytes_to_read == bytes_read);
						luexp(to_file->write(buf.data(), bytes_to_read, &bytes_read));
						luassert(bytes_to_read == bytes_read);
					}
				}
				else
				{
					usize bytes_to_read = (usize)file_sz;
					usize bytes_read;
					luexp(from_file->read(buf.data(), bytes_to_read, &bytes_read));
					luassert(bytes_to_read == bytes_read);
					luexp(to_file->write(buf.data(), bytes_to_read, &bytes_read));
					luassert(bytes_to_read == bytes_read);
				}
			}
			lucatch
			{
				from_file = nullptr;
				to_file = nullptr;
				auto _ = to->m_driver->delete_file(to->m_driver->driver_data, to->m_mount_data, to_path);
				return luerr;
			}
			return ok;
		}
		LUNA_VFS_API R<Ref<IFile>>	open_file(const Path& path, FileOpenFlag flags, FileCreationMode creation)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path relative_path;
			Ref<IFile> ret;
			lutry
			{
				lulet(mnt, route_path(path, relative_path));
				luset(ret, mnt.m_driver->open_file(mnt.m_driver->driver_data, mnt.m_mount_data, relative_path, flags, creation));
			}
			lucatchret;
			return ret;
		}
		LUNA_VFS_API R<FileAttribute> get_file_attribute(const Path& path)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path relative_path;
			FileAttribute ret;
			lutry
			{
				lulet(mnt, route_path(path, relative_path));
				luset(ret, mnt.m_driver->get_file_attribute(mnt.m_driver->driver_data, mnt.m_mount_data, relative_path));
			}
			lucatchret;
			return ret;
		}
		LUNA_VFS_API RV	copy_file(const Path& from_file_path, const Path& to_file_path, FileCopyFlag flags)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path from_path;
			Path to_path;
			lutry
			{
				lulet(from, route_path(from_file_path, from_path));
				lulet(to, route_path(to_file_path, to_path));
				if (from.m_driver == to.m_driver)
				{
					return from.m_driver->copy_file(from.m_driver->driver_data, from.m_mount_data, to.m_mount_data, from_path, to_path, flags);
				}
				// Force copy.
				luexp(copy_file_between_driver(&from, &to, from_path, to_path, test_flags(flags, FileCopyFlag::fail_if_exists)));
			}
			lucatchret;
			return ok;
		}
		LUNA_VFS_API RV	move_file(const Path& from_file_path, const Path& to_file_path, FileMoveFlag flags)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path from_path;
			Path to_path;
			lutry
			{
				lulet(from, route_path(from_file_path, from_path));
				lulet(to, route_path(to_file_path, to_path));
				if (from.m_driver == to.m_driver)
				{
					return from.m_driver->move_file(from.m_driver->driver_data, from.m_mount_data, to.m_mount_data, from_path, to_path, flags);
				}
				// copy and delete.
				luexp(copy_file_between_driver(&from, &to, from_path, to_path, test_flags(flags, FileMoveFlag::fail_if_exists)));
				luexp(from.m_driver->delete_file(from.m_driver->driver_data, from.m_mount_data, from_path));
			}
			lucatchret;
			return ok;
		}
		LUNA_VFS_API RV	delete_file(const Path& file_path)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path relative_path;
			lutry
			{
				lulet(mnt, route_path(file_path, relative_path));
				luexp(mnt.m_driver->delete_file(mnt.m_driver->driver_data, mnt.m_mount_data, relative_path));
			}
			lucatchret;
			return ok;
		}
		LUNA_VFS_API R<Ref<IFileIterator>> open_dir(const Path& dir_path)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path relative_path;
			Ref<IFileIterator> ret;
			lutry
			{
				lulet(mnt, route_path(dir_path, relative_path));
				luset(ret, mnt.m_driver->open_dir(mnt.m_driver->driver_data, mnt.m_mount_data, relative_path));
			}
			lucatchret;
			return ret;
		}
		LUNA_VFS_API RV	create_dir(const Path& dir_path)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path relative_path;
			lutry
			{
				lulet(mnt, route_path(dir_path, relative_path));
				luexp(mnt.m_driver->create_dir(mnt.m_driver->driver_data, mnt.m_mount_data, relative_path));
			}
			lucatchret;
			return ok;
		}
		LUNA_VFS_API R<Name> get_native_path(const Path& vfs_path)
		{
			MutexGuard _guard(g_mounts_mutex);
			Path relative_path;
			Name ret;
			lutry
			{
				lulet(mnt, route_path(vfs_path, relative_path));
				luset(ret, mnt.m_driver->get_native_path(mnt.m_driver->driver_data, mnt.m_mount_data, relative_path));
			}
			lucatchret;
			return ret;
		}

		struct VFSModule : public Module
		{
			virtual const c8* get_name() override { return "VFS"; }
			virtual RV on_init() override
			{
				g_driver_mutex = new_mutex();
				g_mounts_mutex = new_mutex();
				register_platform_filesystem_driver();
				return ok;
			}
			virtual void on_close() override
			{
				g_mounts.clear();
				g_mounts.shrink_to_fit();
				g_mounts_mutex = nullptr;
				for (auto& i : g_drivers)
				{
					if (i.second.get()->driver_close) i.second.get()->driver_close(i.second.get()->driver_data);
				}
				g_drivers.clear();
				g_drivers.shrink_to_fit();
				g_driver_mutex = nullptr;
			}
		};
	}

	LUNA_VFS_API Module* module_vfs()
	{
		static VFS::VFSModule m;
		return &m;
	}

	namespace VFSError
	{
		LUNA_VFS_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("VFSError");
			return e;
		}

		LUNA_VFS_API ErrCode driver_not_found()
		{
			static ErrCode e = get_error_code_by_name("VFSError", "driver_not_found");
			return e;
		}
	}
}