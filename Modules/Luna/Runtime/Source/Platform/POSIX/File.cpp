/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.cpp
* @author JXMaster
* @date 2020/9/27
*/
#include "../../OS.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Algorithm.hpp>

#ifdef LUNA_PLATFORM_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#endif

#include <libgen.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#ifdef LUNA_PLATFORM_MACOS
#include <libproc.h>
#endif

namespace Luna
{
	namespace OS
	{
		struct File
		{
			opaque_t handle;
			bool buffered;
		};

		R<opaque_t> open_unbuffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation)
		{
			lucheck(path);
			int f = 0;
			if ((flags & FileOpenFlag::read) != FileOpenFlag::none)
			{
				if ((flags & FileOpenFlag::write) != FileOpenFlag::none)
				{
					f = O_RDWR;
				}
				else
				{
					f = O_RDONLY;
				}
			}
			else
			{
				if ((flags & FileOpenFlag::write) != FileOpenFlag::none)
				{
					f = O_WRONLY;
				}
				else
				{
					return BasicError::bad_arguments();
				}
			}
			int fd;
			switch (creation)
			{
			case FileCreationMode::create_always:
				f |= O_CREAT | O_TRUNC;
				fd = open(path, f, 0666);
				break;
			case FileCreationMode::create_new:
				if (get_file_attribute(path).valid())
				{
					return BasicError::already_exists();
				}
				f |= O_CREAT;
				fd = open(path, f, 0666);
				break;
			case FileCreationMode::open_always:
				f |= O_CREAT;
				fd = open(path, f, 0666);
				break;
			case FileCreationMode::open_existing:
				fd = open(path, f, 0);
				break;
			case FileCreationMode::open_existing_as_new:
				f |= O_TRUNC;
				fd = open(path, f, 0);
				break;
			}
			if (fd == -1)
			{
				auto err = errno;
				switch (err)
				{
				case EPERM:
					return BasicError::access_denied();
				case ENOENT:
					return BasicError::not_found();
				default:
					return BasicError::bad_platform_call();
				}
			}
			return (opaque_t)(usize)fd;
		}
		void close_unbuffered_file(opaque_t file)
		{
			int fd = (int)(usize)file;
			::close(fd);
		}
		RV read_unbuffered_file(opaque_t file, void* buffer, usize size, usize* read_bytes)
		{
			int fd = (int)(usize)file;
			isize sz = ::read(fd, buffer, size);
			if (sz == -1)
			{
				if (read_bytes)
				{
					*read_bytes = 0;
				}
				return BasicError::bad_platform_call();
			}
			if (read_bytes)
			{
				*read_bytes = sz;
			}
			return ok;
		}
		RV write_unbuffered_file(opaque_t file, const void* buffer, usize size, usize* write_bytes)
		{
			int fd = (int)(usize)file;
			isize sz = ::write(fd, buffer, size);
			if (sz == -1)
			{
				if (write_bytes)
				{
					*write_bytes = 0;
				}
				return BasicError::bad_platform_call();
			}
			if (write_bytes)
			{
				*write_bytes = sz;
			}
			return ok;
		}
		u64 get_unbuffered_file_size(opaque_t file)
		{
			int fd = (int)(usize)file;
			struct stat st;
			if (!fstat(fd, &st))
			{
				return st.st_size;
			}
			lupanic_msg_always("fstat failed.");
			return 0;
		}
		RV set_unbuffered_file_size(opaque_t file, u64 sz)
		{
			int fd = (int)(usize)file;
			return ftruncate(fd, sz) ? BasicError::bad_platform_call() : RV();
		}
		R<u64> get_unbuffered_file_cursor(opaque_t file)
		{
			int fd = (int)(usize)file;
			off_t r = lseek(fd, 0, SEEK_CUR);
			if (r == (off_t)-1)
			{
				return BasicError::bad_platform_call();
			}
			return(u64)r;
		}
		RV set_unbuffered_file_cursor(opaque_t file, i64 offset, SeekMode mode)
		{
			int fd = (int)(usize)file;
			int origin;
			switch (mode)
			{
			case SeekMode::begin:
				origin = SEEK_SET;
				break;
			case SeekMode::current:
				origin = SEEK_CUR;
				break;
			case SeekMode::end:
				origin = SEEK_END;
				break;
			}
			off_t r = lseek(fd, offset, origin);
			if (r == (off_t)-1)
			{
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		void flush_unbuffered_file(opaque_t file)
		{
			int fd = (int)(usize)file;
			fsync(fd);
		}
		R<opaque_t> open_buffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation)
		{
			// use buffered version.
			const char* mode;
			FILE* f = NULL;
			auto err = errno;
			if (((flags & FileOpenFlag::read) != FileOpenFlag::none) && ((flags & FileOpenFlag::write) != FileOpenFlag::none))
			{
				// update mode.
				switch (creation)
				{
				case FileCreationMode::create_always:
					mode = "w+b";
					f = fopen(path, mode);
					break;
				case FileCreationMode::create_new:
					if (get_file_attribute(path).valid())
					{
						return BasicError::already_exists();
					}
					mode = "w+b";
					f = fopen(path, mode);
					break;
				case FileCreationMode::open_always:
					if (get_file_attribute(path).valid())
					{
						mode = "r+b";
						f = fopen(path, mode);
					}
					else
					{
						mode = "w+b";
						f = fopen(path, mode);
					}
					break;
				case FileCreationMode::open_existing:
					mode = "r+b";
					f = fopen(path, mode);
					break;
				case FileCreationMode::open_existing_as_new:
					if (get_file_attribute(path).valid())
					{
						mode = "w+b";
						f = fopen(path, mode);
					}
					else
					{
						return BasicError::not_found();
					}
					break;
				default:
					lupanic();
					break;
				}
			}
			else if (((flags & FileOpenFlag::read) != FileOpenFlag::none))
			{
				// read only mode,
				switch (creation)
				{
				case FileCreationMode::create_always:
				case FileCreationMode::create_new:
				case FileCreationMode::open_existing_as_new:
				case FileCreationMode::open_always:
					return BasicError::bad_arguments();    // Creates a new empty file and read-only from it has no meaning.
					break;
				case FileCreationMode::open_existing:
					mode = "rb";
					f = fopen(path, mode);
					break;
				default:
					lupanic();
					break;
				}
			}
			else if (((flags & FileOpenFlag::write) != FileOpenFlag::none))
			{
				// write only mode.
				switch (creation)
				{
				case FileCreationMode::create_always:
					mode = "wb";
					f = fopen(path, mode);
					break;
				case FileCreationMode::create_new:
					if (get_file_attribute(path).valid())
					{
						return BasicError::already_exists();
					}
					mode = "wb";
					f = fopen(path, mode);
					break;
				case FileCreationMode::open_always:
					if (get_file_attribute(path).valid())
					{
						mode = "r+b";
						f = fopen(path, mode);
					}
					else
					{
						mode = "wb";
						f = fopen(path, mode);
					}
					break;
				case FileCreationMode::open_existing:
					mode = "r+b";
					f = fopen(path, mode);
					break;
				case FileCreationMode::open_existing_as_new:
					if (get_file_attribute(path).valid())
					{
						mode = "wb";
						f = fopen(path, mode);
					}
					else
					{
						return BasicError::not_found();
					}
					break;
				default:
					lupanic();
					break;
				}
			}
			if (!f)
			{
				err = errno;
				switch (err)
				{
				case EPERM:
					return BasicError::access_denied();
				case ENOENT:
					return BasicError::not_found();
				default:
					//get_error().set(e_bad_system_call, "fopen failed with err code %d", err);
					return BasicError::bad_platform_call();
				}
			}
			return f;
		}
		void close_buffered_file(opaque_t file)
		{
			FILE* f = (FILE*)file;
			fclose(f);
		}
		RV read_buffered_file(opaque_t file, void* buffer, usize size, usize* read_bytes)
		{
			FILE* f = (FILE*)file;
			usize sz = fread(buffer, 1, size, f);
			if (read_bytes)
			{
				*read_bytes = sz;
			}
			if (sz != size)
			{
				if (feof(f))
				{
					clearerr(f);
					return ok;
				}
				clearerr(f);
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		RV write_buffered_file(opaque_t file, const void* buffer, usize size, usize* write_bytes)
		{
			FILE* f = (FILE*)file;
			usize sz = fwrite(buffer, 1, size, f);
			if (write_bytes)
			{
				*write_bytes = sz;
			}
			if (sz != size)
			{
				clearerr(f);
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		u64 get_buffered_file_size(opaque_t file)
		{
			FILE* f = (FILE*)file;
			int fd = fileno(f);
			struct stat st;
			if (!fstat(fd, &st))
			{
				return st.st_size;
			}
			lupanic_msg_always("fstat failed");
			return 0;
		}
		RV set_buffered_file_size(opaque_t file, u64 sz)
		{
			FILE* f = (FILE*)file;
			int fd = fileno(f);
			return ftruncate(fd, sz) ? BasicError::bad_platform_call() : RV();
		}
		R<u64> get_buffered_file_cursor(opaque_t file)
		{
			FILE* f = (FILE*)file;
			long r = ftell(f);
			if (r < 0)
			{
				clearerr(f);
				return BasicError::bad_platform_call();
			}
			return (u64)r;
		}
		RV set_buffered_file_cursor(opaque_t file, i64 offset, SeekMode mode)
		{
			FILE* f = (FILE*)file;
			int origin;
			switch (mode)
			{
			case SeekMode::begin:
				origin = SEEK_SET;
				break;
			case SeekMode::current:
				origin = SEEK_CUR;
				break;
			case SeekMode::end:
				origin = SEEK_END;
				break;
			}
			if (fseek(f, offset, origin))
			{
				clearerr(f);
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		void flush_buffered_file(opaque_t file)
		{
			FILE* f = (FILE*)file;
			if (fflush(f))
			{
				lupanic_msg_always("fflush failed.");
			}
		}
		R<opaque_t> open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation)
		{
			bool buffered = test_flags(flags, FileOpenFlag::user_buffering);
			R<opaque_t> f = nullptr;
			if (buffered)
			{
				f = open_buffered_file(path, flags, creation);
			}
			else
			{
				f = open_unbuffered_file(path, flags, creation);
			}
			if (failed(f))
			{
				return f;
			}
			File* ret = Luna::memnew<File>();
			ret->buffered = buffered;
			ret->handle = f.get();
			return ret;
		}
		void close_file(opaque_t file)
		{
			File* f = (File*)file;
			if (f->buffered) close_buffered_file(f->handle);
			else close_unbuffered_file(f->handle);
			Luna::memdelete(f);
		}
		RV read_file(opaque_t file, void* buffer, usize size, usize* read_bytes)
		{
			File* f = (File*)file;
			return f->buffered ? read_buffered_file(f->handle, buffer, size, read_bytes) :
				read_unbuffered_file(f->handle, buffer, size, read_bytes);
		}
		RV write_file(opaque_t file, const void* buffer, usize size, usize* write_bytes)
		{
			File* f = (File*)file;
			return f->buffered ? write_buffered_file(f->handle, buffer, size, write_bytes) :
				write_unbuffered_file(f->handle, buffer, size, write_bytes);
		}
		u64 get_file_size(opaque_t file)
		{
			File* f = (File*)file;
			return f->buffered ? get_buffered_file_size(f->handle) : get_unbuffered_file_size(f->handle);
		}
		RV set_file_size(opaque_t file, u64 sz)
		{
			File* f = (File*)file;
			return f->buffered ? set_buffered_file_size(f->handle, sz) : set_unbuffered_file_size(f->handle, sz);
		}
		R<u64> get_file_cursor(opaque_t file)
		{
			File* f = (File*)file;
			return f->buffered ? get_buffered_file_cursor(f->handle) : get_unbuffered_file_cursor(f->handle);
		}
		RV set_file_cursor(opaque_t file, i64 offset, SeekMode mode)
		{
			File* f = (File*)file;
			return f->buffered ? set_buffered_file_cursor(f->handle, offset, mode) : set_unbuffered_file_cursor(f->handle, offset, mode);
		}
		void flush_file(opaque_t file)
		{
			File* f = (File*)file;
			if (f->buffered) flush_buffered_file(f->handle);
			else flush_unbuffered_file(f->handle);
		}
		R<FileAttribute> get_file_attribute(const c8* path)
		{
			struct stat s;
			int r = stat(path, &s);
			if (r != 0)
			{
				return BasicError::bad_platform_call();
			}
			FileAttribute attribute;
			attribute.size = s.st_size;
			attribute.last_access_time = s.st_atime;
			attribute.last_write_time = s.st_mtime;
#ifdef LUNA_PLATFORM_MACOS
			attribute.creation_time = s.st_birthtime;
#else
			attribute.creation_time = 0;
#endif
			attribute.attributes = FileAttributeFlag::none;
			if (S_ISDIR(s.st_mode))
			{
				attribute.attributes |= FileAttributeFlag::directory;
			}
			if (S_ISCHR(s.st_mode))
			{
				attribute.attributes |= FileAttributeFlag::character_special;
			}
			if (S_ISBLK(s.st_mode))
			{
				attribute.attributes |= FileAttributeFlag::block_special;
			}
			return attribute;
		}
        RV copy_file(const c8* from_path, const c8* to_path, FileCopyFlag flags)
		{
			lucheck(from_path && to_path);
			constexpr u64 max_buffer_sz = 1_mb;
			u8* buf = (u8*)memalloc(max_buffer_sz);
			opaque_t from_file = nullptr;
			opaque_t to_file = nullptr;
			lutry
			{
				luset(from_file, OS::open_file(from_path, FileOpenFlag::read, FileCreationMode::open_existing));
				if (test_flags(flags, FileCopyFlag::fail_if_exists))
				{
					luset(to_file, OS::open_file(to_path, FileOpenFlag::write, FileCreationMode::create_new));
				}
				else
				{
					luset(to_file, OS::open_file(to_path, FileOpenFlag::write, FileCreationMode::create_always));
				}
				auto copy_size = OS::get_file_size(from_file);
				u64 sz = copy_size;
				while (sz)
				{
					usize copy_size_onetime = min<usize>(sz, max_buffer_sz);
					luexp(read_file(from_file, buf, copy_size_onetime, nullptr));
					luexp(write_file(to_file, buf, copy_size_onetime, nullptr));
					sz -= copy_size_onetime;
				}
			}
			lucatch
			{
				if (buf)
				{
					memfree(buf);
					buf = nullptr;
				}
				if (from_file)
				{
					close_file(from_file);
				}
				if (to_file)
				{
					close_file(to_file);
				}
				return luerr;
			}
				if (buf)
				{
					memfree(buf);
					buf = nullptr;
				}
			if (from_file)
			{
				close_file(from_file);
			}
			if (to_file)
			{
				close_file(to_file);
			}
			return ok;
		}
        RV move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags)
		{
            bool fail_if_exists = test_flags(flags, FileMoveFlag::fail_if_exists);
			if (fail_if_exists && get_file_attribute(to_path).valid())
			{
				return BasicError::already_exists();
			}
			int res = ::rename(from_path, to_path);
			if (res != 0)
			{
				// Try to copy&delete.
				lutry
				{
					luexp(OS::copy_file(from_path, to_path, fail_if_exists? FileCopyFlag::fail_if_exists : FileCopyFlag::none));
					luexp(OS::delete_file(from_path));
				}
				lucatchret;
			}
			return ok;
		}
        RV delete_file(const c8* path)
		{
			int res = ::remove(path);
			return (res == 0) ? RV() : BasicError::bad_platform_call();
		}
		struct FileData
		{
			DIR* m_dir;
			struct dirent* m_dirent;
        };
		R<opaque_t> open_dir(const c8* path)
		{
			DIR* dir = ::opendir(path);
			if (dir == NULL)
			{
				auto err = errno;
				switch (err)
				{
				case EACCES:
					return BasicError::access_denied();
				case EMFILE:
				case ENFILE:
					return BasicError::out_of_resource();
				case ENOENT:
					return BasicError::not_found();
				case ENOMEM:
					return BasicError::out_of_memory();
				case ENOTDIR:
					return BasicError::not_directory();
				default:
					return BasicError::bad_platform_call();
				}
			}
			FileData* iter = memnew<FileData>();
			iter->m_dir = dir;
			iter->m_dirent = ::readdir(dir);
			return iter;
		}
		void close_dir(opaque_t dir_iter)
		{
			FileData* data = (FileData*)dir_iter;
			closedir(data->m_dir);
			memdelete(data);
		}
		bool dir_iterator_is_valid(opaque_t dir_iter)
		{
			FileData* data = (FileData*)dir_iter;
			return data->m_dirent != nullptr;
		}
		const c8* dir_iterator_get_filename(opaque_t dir_iter)
		{
			FileData* data = (FileData*)dir_iter;
			if (data->m_dirent)
			{
				return data->m_dirent->d_name;
			}
			return nullptr;
		}
		FileAttributeFlag dir_iterator_get_attributes(opaque_t dir_iter)
		{
			FileData* data = (FileData*)dir_iter;
			FileAttributeFlag flags = FileAttributeFlag::none;
			if (data->m_dirent)
			{
				if (DT_BLK & data->m_dirent->d_type)
				{
					flags |= FileAttributeFlag::block_special;
				}
				if (DT_CHR & data->m_dirent->d_type)
				{
					flags |= FileAttributeFlag::character_special;
				}
				if (DT_DIR & data->m_dirent->d_type)
				{
					flags |= FileAttributeFlag::directory;
				}	
			}
			return flags;
		}
		bool dir_iterator_move_next(opaque_t dir_iter)
		{
			FileData* data = (FileData*)dir_iter;
			if (data->m_dirent)
			{
				data->m_dirent = ::readdir(data->m_dir);
			}
			return data->m_dirent != nullptr;
		}
		RV create_dir(const c8* path)
		{
			int r = mkdir(path, 0755);
			if (r != 0)
			{
				auto err = errno;
				switch (errno)
				{
				case EACCES:
					return BasicError::access_denied();
				case EEXIST:
					return BasicError::already_exists();
				case ENAMETOOLONG:
					return BasicError::data_too_long();
				case ENOENT:
					return BasicError::not_found();
				case ENOTDIR:
					return BasicError::not_directory();
				default:
					return BasicError::bad_platform_call();
				}
			}
			return ok;
		}
		RV remove_dir(const c8* path)
		{
			int r = rmdir(path);
			if (r != 0)
			{
				auto err = errno;
				switch (errno)
				{
				case EACCES:
					return BasicError::access_denied();
				case EBUSY:
					return BasicError::not_ready();
				case ENAMETOOLONG:
					return BasicError::data_too_long();
				case ENOENT:
					return BasicError::not_found();
				case ENOTDIR:
					return BasicError::not_directory();
				default:
					return BasicError::bad_platform_call();
				}
			}
			return ok;
		}
		u32 get_current_dir(u32 buffer_length, c8* buffer)
		{
			char* path = ::getcwd(nullptr, 0);
			u32 len = (u32)strlen(path);
			if (buffer && buffer_length)
			{
				strncpy(buffer, path, buffer_length);
			}
			::free(path);
			return len;
		}
		RV set_current_dir(const c8* path)
		{
			int r = ::chdir(path);
			if (r != 0)
			{
				auto err = errno;
				switch (errno)
				{
				case EACCES:
					return BasicError::access_denied();
				case ENAMETOOLONG:
					return BasicError::data_too_long();
				case ENOENT:
					return BasicError::not_found();
				case ENOTDIR:
					return BasicError::not_directory();
				default:
					return BasicError::bad_platform_call();
				}
			}
			return ok;
		}
		c8 g_process_path[1024];

		void file_init()
		{
#ifdef LUNA_PLATFORM_LINUX
			char path[1024];
			luassert_always(readlink("/proc/self/exe", path, 1024) != -1);
			char* dir = dirname(path);
			strcpy(g_process_path, dir);
			g_process_path[1023] = 0;
#else
			pid_t pid = getpid();
			int ret = proc_pidpath(pid, g_process_path, sizeof(g_process_path));
			luassert_always(ret > 0);
#endif
		}

		const c8* get_process_path()
		{
			return g_process_path;
		}
    }
}