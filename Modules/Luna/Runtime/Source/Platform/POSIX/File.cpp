/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.cpp
* @author JXMaster
* @date 2020/9/27
*/
#include "../File.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Algorithm.hpp>
#include "Errno.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <libgen.h>
#include <errno.h>

#include <fcntl.h>
#if defined(LUNA_PLATFORM_LINUX)
#define _GNU_SOURCE
#endif
#include <unistd.h>

#if defined(LUNA_PLATFORM_APPLE)
#include <copyfile.h>
#include <mach-o/dyld.h>
#endif

namespace Luna
{
    namespace Platform
    {
        Result open_unbuffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, int& out_fd)
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
                    return Result::bad_arguments;
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
                {
                    FileAttribute attr;
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        return Result::already_exists;
                    }
                    f |= O_CREAT;
                    fd = open(path, f, 0666);
                }
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
                return encode_errno(errno);
            }
            out_fd = fd;
            return Result::success;
        }
        void close_unbuffered_file(int fd)
        {
            ::close(fd);
        }
        Result read_unbuffered_file(int fd, void* buffer, usize size, usize* read_bytes)
        {
            isize sz = ::read(fd, buffer, size);
            if (sz == -1)
            {
                if (read_bytes)
                {
                    *read_bytes = 0;
                }
                return Result::bad_platform_call;
            }
            if (read_bytes)
            {
                *read_bytes = sz;
            }
            return Result::success;
        }
        Result write_unbuffered_file(int fd, const void* buffer, usize size, usize* write_bytes)
        {
            isize sz = ::write(fd, buffer, size);
            if (sz == -1)
            {
                if (write_bytes)
                {
                    *write_bytes = 0;
                }
                return Result::bad_platform_call;
            }
            if (write_bytes)
            {
                *write_bytes = sz;
            }
            return Result::success;
        }
        u64 get_unbuffered_file_size(int fd)
        {
            struct stat st;
            if (!fstat(fd, &st))
            {
                return st.st_size;
            }
            lupanic_msg_always("fstat failed.");
            return 0;
        }
        Result set_unbuffered_file_size(int fd, u64 sz)
        {
            return ftruncate(fd, sz) ? Result::bad_platform_call : Result::success;
        }
        Result get_unbuffered_file_cursor(int fd, u64& out_cursor)
        {
            off_t r = lseek(fd, 0, SEEK_CUR);
            if (r == (off_t)-1)
            {
                return Result::bad_platform_call;
            }
            out_cursor = (u64)r;
            return Result::success;
        }
        Result set_unbuffered_file_cursor(int fd, i64 offset, SeekMode mode)
        {
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
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        void flush_unbuffered_file(int fd)
        {
            fsync(fd);
        }
        Result open_buffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, FILE*& out_file)
        {
            // use buffered version.
            const char* mode;
            FILE* f = NULL;
            FileAttribute attr;
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
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        return Result::already_exists;
                    }
                    mode = "w+b";
                    f = fopen(path, mode);
                    break;
                case FileCreationMode::open_always:
                    if (get_file_attribute(path, attr) == Result::success)
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
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        mode = "w+b";
                        f = fopen(path, mode);
                    }
                    else
                    {
                        return Result::not_found;
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
                    return Result::bad_arguments;    // Creates a new empty file and read-only from it has no meaning.
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
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        return Result::already_exists;
                    }
                    mode = "wb";
                    f = fopen(path, mode);
                    break;
                case FileCreationMode::open_always:
                    if (get_file_attribute(path, attr) == Result::success)
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
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        mode = "wb";
                        f = fopen(path, mode);
                    }
                    else
                    {
                        return Result::not_found;
                    }
                    break;
                default:
                    lupanic();
                    break;
                }
            }
            if (!f)
            {
                return encode_errno(errno);
            }
            out_file = f;
            return Result::success;
        }
        void close_buffered_file(FILE* f)
        {
            fclose(f);
        }
        Result read_buffered_file(FILE* f, void* buffer, usize size, usize* read_bytes)
        {
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
                    return Result::success;
                }
                clearerr(f);
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        Result write_buffered_file(FILE* f, const void* buffer, usize size, usize* write_bytes)
        {
            usize sz = fwrite(buffer, 1, size, f);
            if (write_bytes)
            {
                *write_bytes = sz;
            }
            if (sz != size)
            {
                clearerr(f);
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        u64 get_buffered_file_size(FILE* f)
        {
            int fd = fileno(f);
            struct stat st;
            if (!fstat(fd, &st))
            {
                return st.st_size;
            }
            lupanic_msg_always("fstat failed");
            return 0;
        }
        Result set_buffered_file_size(FILE* f, u64 sz)
        {
            int fd = fileno(f);
            return ftruncate(fd, sz) ? Result::bad_platform_call : Result();
        }
        Result get_buffered_file_cursor(FILE* f, u64& out_cursor)
        {
            long r = ftell(f);
            if (r < 0)
            {
                clearerr(f);
                return Result::bad_platform_call;
            }
            out_cursor = (u64)r;
            return Result::success;
        }
        Result set_buffered_file_cursor(FILE* f, i64 offset, SeekMode mode)
        {
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
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        void flush_buffered_file(FILE* f)
        {
            if (fflush(f))
            {
                lupanic_msg_always("fflush failed.");
            }
        }
        Result open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, File& out_file)
        {
            bool buffered = test_flags(flags, FileOpenFlag::user_buffering);
            out_file.m_buffered = buffered;
            Result r;
            if (buffered)
            {
                FILE* f = nullptr;
                r = open_buffered_file(path, flags, creation, f);
                out_file.m_file = f;
            }
            else
            {
                int fd = 0;
                r = open_unbuffered_file(path, flags, creation, fd);
                out_file.m_fd = fd;
            }
            return r;
        }
        void close_file(File& file)
        {
            if (file.m_buffered) close_buffered_file(file.m_file);
            else close_unbuffered_file(file.m_fd);
        }
        Result read_file(File& file, void* buffer, usize size, usize* read_bytes)
        {
            return file.m_buffered ? read_buffered_file(file.m_file, buffer, size, read_bytes) :
                read_unbuffered_file(file.m_fd, buffer, size, read_bytes);
        }
        Result write_file(File& file, const void* buffer, usize size, usize* write_bytes)
        {
            return file.m_buffered ? write_buffered_file(file.m_file, buffer, size, write_bytes) :
                write_unbuffered_file(file.m_fd, buffer, size, write_bytes);
        }
        u64 get_file_size(File& file)
        {
            return file.m_buffered ? get_buffered_file_size(file.m_file) : get_unbuffered_file_size(file.m_fd);
        }
        Result set_file_size(File& file, u64 sz)
        {
            return file.m_buffered ? set_buffered_file_size(file.m_file, sz) : set_unbuffered_file_size(file.m_fd, sz);
        }
        Result get_file_cursor(File& file, u64& out_cursor)
        {
            return file.m_buffered ? get_buffered_file_cursor(file.m_file, out_cursor) : get_unbuffered_file_cursor(file.m_fd, out_cursor);
        }
        Result set_file_cursor(File& file, i64 offset, SeekMode mode)
        {
            return file.m_buffered ? set_buffered_file_cursor(file.m_file, offset, mode) : set_unbuffered_file_cursor(file.m_fd, offset, mode);
        }
        void flush_file(File& file)
        {
            if (file.m_buffered) flush_buffered_file(file.m_file);
            else flush_unbuffered_file(file.m_fd);
        }
        Result get_file_attribute(const c8* path, FileAttribute& out_attribute)
        {
            struct stat s;
            int r = stat(path, &s);
            if (r != 0)
            {
                return encode_errno(errno);
            }
            out_attribute.size = s.st_size;
            out_attribute.last_access_time = s.st_atime;
            out_attribute.last_write_time = s.st_mtime;
#ifdef LUNA_PLATFORM_MACOS
            out_attribute.creation_time = s.st_birthtime;
#else
            out_attribute.creation_time = 0;
#endif
            out_attribute.attributes = FileAttributeFlag::none;
            if (S_ISDIR(s.st_mode))
            {
                out_attribute.attributes |= FileAttributeFlag::directory;
            }
            if (S_ISCHR(s.st_mode))
            {
                out_attribute.attributes |= FileAttributeFlag::character_special;
            }
            if (S_ISBLK(s.st_mode))
            {
                out_attribute.attributes |= FileAttributeFlag::block_special;
            }
            return Result::success;
        }
        Result copy_file(const c8* from_path, const c8* to_path)
        {
            lucheck(from_path && to_path);
            FileAttribute attr;
            if (get_file_attribute(from_path, attr) != Result::success)
            {
                return Result::not_found;
            }
            if ((attr.attributes & FileAttributeFlag::directory) != FileAttributeFlag::none)
            {
                return Result::is_directory;
            }
            if (get_file_attribute(to_path, attr) == Result::success)
            {
                return Result::already_exists;
            }
            int src_fd = open(from_path, O_RDONLY, 0);
            if (src_fd == -1)
            {
                return encode_errno(errno);
            }
            int dst_fd = open(to_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (dst_fd == -1)
            {
                close(src_fd);
                return encode_errno(errno);
            }
            Result res = Result::success;
#if defined(LUNA_PLATFORM_LINUX)
            // copy_file_range: in-kernel copy, no user-space round-trip (Linux 4.5+, glibc 2.27+ / bionic)
            u64 total_copied = 0;
            u64 file_size = attr.size;
            while (total_copied < file_size)
            {
                size_t remaining = (size_t)(file_size - total_copied);
                if (file_size - total_copied > (u64)(size_t)-1)
                    remaining = (size_t)-1;
                ssize_t n = copy_file_range(src_fd, nullptr, dst_fd, nullptr, remaining, 0);
                if (n < 0)
                {
                    res = encode_errno(errno);
                    break;
                }
                if (n == 0)
                    break;
                total_copied += (u64)n;
            }
#elif defined(LUNA_PLATFORM_ANDROID)
            c8 buffer[64 * 1024];
            for(;;)
            {
                ssize_t bytes_read = read(src_fd, buffer, sizeof(buffer));
                if(bytes_read < 0)
                {
                    res = encode_errno(errno);
                    break;
                }
                if(bytes_read == 0)
                {
                    break;
                }

                c8* cursor = buffer;
                ssize_t bytes_remaining = bytes_read;
                while(bytes_remaining > 0)
                {
                    ssize_t bytes_written = write(dst_fd, cursor, (usize)bytes_remaining);
                    if(bytes_written < 0)
                    {
                        res = encode_errno(errno);
                        break;
                    }
                    cursor += bytes_written;
                    bytes_remaining -= bytes_written;
                }
                if(res != Result::success)
                {
                    break;
                }
            }
#elif defined(LUNA_PLATFORM_APPLE)
            // fcopyfile: native copy on macOS/iOS (copyfile.h)
            if (fcopyfile(src_fd, dst_fd, nullptr, COPYFILE_DATA) != 0)
            {
                res = encode_errno(errno);
            }
#else
#error "copy_file: unsupported POSIX platform (only Linux, Android, macOS, iOS are supported)"
#endif
            close(src_fd);
            close(dst_fd);
            if (res != Result::success)
            {
                ::remove(to_path);
            }
            return res;
        }
        Result move_file(const c8* from_path, const c8* to_path)
        {
            FileAttribute attr;
            if (get_file_attribute(to_path, attr) == Result::success)
            {
                return Result::already_exists;
            }
            int res = ::rename(from_path, to_path);
            if(res != 0)
            {
                return encode_errno(errno);
            }
            return Result::success;
        }
        Result delete_file(const c8* path)
        {
            int res = ::remove(path);
            if(res != 0)
            {
                return encode_errno(errno);
            }
            return Result::success;
        }
        Result open_dir(const c8* path, FileIterator& out_dir_iter)
        {
            DIR* dir = ::opendir(path);
            if (dir == NULL)
            {
                return encode_errno(errno);
            }
            out_dir_iter.m_dir = dir;
            out_dir_iter.m_dirent = ::readdir(dir);
            return Result::success;
        }
        void close_dir(FileIterator& dir_iter)
        {
            closedir(dir_iter.m_dir);
            dir_iter.m_dir = nullptr;
        }
        bool dir_iterator_is_valid(FileIterator& dir_iter)
        {
            return dir_iter.m_dirent != nullptr;
        }
        const c8* dir_iterator_get_filename(FileIterator& dir_iter)
        {
            if (dir_iter.m_dirent)
            {
                return dir_iter.m_dirent->d_name;
            }
            return nullptr;
        }
        FileAttributeFlag dir_iterator_get_attributes(FileIterator& dir_iter)
        {
            FileAttributeFlag flags = FileAttributeFlag::none;
            if (dir_iter.m_dirent)
            {
                if (DT_BLK & dir_iter.m_dirent->d_type)
                {
                    flags |= FileAttributeFlag::block_special;
                }
                if (DT_CHR & dir_iter.m_dirent->d_type)
                {
                    flags |= FileAttributeFlag::character_special;
                }
                if (DT_DIR & dir_iter.m_dirent->d_type)
                {
                    flags |= FileAttributeFlag::directory;
                }    
            }
            return flags;
        }
        bool dir_iterator_move_next(FileIterator& dir_iter)
        {
            if (dir_iter.m_dirent)
            {
                dir_iter.m_dirent = ::readdir(dir_iter.m_dir);
            }
            return dir_iter.m_dirent != nullptr;
        }
        Result create_dir(const c8* path)
        {
            int r = mkdir(path, 0755);
            if (r != 0)
            {
                return encode_errno(errno);
            }
            return Result::success;
        }
        Result remove_dir(const c8* path)
        {
            int r = rmdir(path);
            if (r != 0)
            {
                return encode_errno(errno);
            }
            return Result::success;
        }
        const c8* get_current_dir()
        {
            return ::getcwd(nullptr, 0);
        }
        void release_current_dir(const c8* path)
        {
            ::free((void*)path);
        }
        Result set_current_dir(const c8* path)
        {
            int r = ::chdir(path);
            if (r != 0)
            {
                return encode_errno(errno);
            }
            return Result::success;
        }

        const c8* get_process_path()
        {
#if defined(LUNA_PLATFORM_LINUX)
            static_assert(sizeof(c8) == sizeof(char), "Unsupported char size");
            usize buf_size = 128;
            usize read_size = 0;
            char* path = (char*)memalloc(buf_size);
            while(true)
            {
                auto r = readlink("/proc/self/exe", path, buf_size);
                luassert_msg_always(r != -1, "readlink failed");
                read_size = (usize)r;
                if(read_size < buf_size)
                {
                    break;
                }
                buf_size += 128;
                path = (char*)memrealloc(path, buf_size);
            }
            path[read_size] = 0;
            return path;
#elif defined(LUNA_PLATFORM_APPLE)
            uint32_t buf_size = 0;
            _NSGetExecutablePath(NULL, &buf_size);
            c8* path = (c8*)memalloc(buf_size);
            int r = _NSGetExecutablePath(path, &buf_size);
            luassert_msg_always(r == 0, "_NSGetExecutablePath failed");
            return path;
#endif
        }

        void release_process_path(const c8* path)
        {
            memfree((void*)path);
        }
    }
}
