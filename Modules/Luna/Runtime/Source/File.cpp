/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.cpp
* @author JXMaster
* @date 2019/9/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "File.hpp"

namespace Luna
{
	LUNA_RUNTIME_API R<Ref<IFile>>	open_file(const c8* filename, FileOpenFlag flags, FileCreationMode creation)
	{
		Ref<IFile> ret;
		lutry
		{
			lulet(file_handle, OS::open_file(filename, flags, creation));
			auto file = new_object<File>();
			file->m_file = file_handle;
			ret = file;
		}
#ifdef LUNA_DEBUG
		lucatch
		{
			return set_error(luerr, "Failed to open file %s: %s", filename, explain(luerr));
		}
#else
		lucatchret;
#endif
		return ret;
	}
	LUNA_RUNTIME_API R<Blob> load_file_data(IFile* file)
	{
		Blob ret;
		lutry
		{
			lulet(cursor, file->tell());
			luexp(file->seek(0, SeekMode::begin));
			ret.resize((usize)file->get_size());
			luexp(file->read(ret.data(), ret.size()));
			luexp(file->seek((i64)cursor, SeekMode::begin));
		}
		lucatchret;
		return ret;
	}
	LUNA_RUNTIME_API R<FileAttribute> get_file_attribute(const c8* filename)
	{
		return OS::get_file_attribute(filename);
	}
	LUNA_RUNTIME_API RV	copy_file(const c8* from_path, const c8* to_path, FileCopyFlag flags)
	{
		return OS::copy_file(from_path, to_path, flags);
	}
	LUNA_RUNTIME_API RV	move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags)
	{
		return OS::move_file(from_path, to_path, flags);
	}
	LUNA_RUNTIME_API RV	delete_file(const c8* filename)
	{
		return OS::delete_file(filename);
	}
	LUNA_RUNTIME_API R<Ref<IFileIterator>> open_dir(const c8* dir_path)
	{
		Ref<IFileIterator> ret;
		lutry
		{
			lulet(handle, OS::open_dir(dir_path));
			auto iter = new_object<FileIterator>();
			iter->m_handle = handle;
			ret = iter;
		}
		lucatchret;
		return ret;
	}
	LUNA_RUNTIME_API RV	create_dir(const c8* pathname)
	{
		return OS::create_dir(pathname);
	}
	LUNA_RUNTIME_API u32 get_current_dir(u32 buffer_length, c8* buffer)
	{
		return OS::get_current_dir(buffer_length, buffer);
	}
	LUNA_RUNTIME_API RV set_current_dir(const c8* path)
	{
		return OS::set_current_dir(path);
	}
	LUNA_RUNTIME_API const c8* get_process_path()
	{
		return OS::get_process_path();
	}
}