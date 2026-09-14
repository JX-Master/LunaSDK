/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.cpp
* @author JXMaster
* @date 2019/9/29
*/
#include "../../../Platform/Windows/MiniWin.hpp"
#include <io.h>
#include <Luna/Runtime/Unicode.hpp>
#include "../File.hpp"
#include <shellapi.h>
#include "ErrCode.hpp"
#include "Utils.hpp"

#pragma comment(lib, "Shell32.lib")

namespace Luna
{
    namespace Platform
    {
        Result open_unbuffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, HANDLE& out_handle)
        {
            lucheck(path);

            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            DWORD dw_access = 0;
            DWORD dw_creation = 0;
            if ((flags & FileOpenFlag::read) != FileOpenFlag::none)
            {
                dw_access |= GENERIC_READ;
            }
            if ((flags & FileOpenFlag::write) != FileOpenFlag::none)
            {
                dw_access |= GENERIC_WRITE;
            }
            switch (creation)
            {
            case FileCreationMode::create_always:
                dw_creation = CREATE_ALWAYS;
                break;
            case FileCreationMode::create_new:
                dw_creation = CREATE_NEW;
                break;
            case FileCreationMode::open_always:
                dw_creation = OPEN_ALWAYS;
                break;
            case FileCreationMode::open_existing:
                dw_creation = OPEN_EXISTING;
                break;
            case FileCreationMode::open_existing_as_new:
                dw_creation = TRUNCATE_EXISTING;
                break;
            default:
                lupanic();
                break;
            }
            out_handle = ::CreateFileW(pathbuffer, dw_access, FILE_SHARE_READ, nullptr, dw_creation, FILE_ATTRIBUTE_NORMAL, nullptr);
            memfree(pathbuffer);
            if (out_handle == INVALID_HANDLE_VALUE)
            {
                DWORD dw = ::GetLastError();
                return translate_last_error(dw);
            }
            return Result::success;
        }
        void close_unbuffered_file(HANDLE file)
        {
            ::CloseHandle(file);
        }
        Result read_unbuffered_file(HANDLE file, void* buffer, usize size, usize* read_bytes)
        {
            luassert(file);
            DWORD actual = 0;
            BOOL s = ::ReadFile(file, buffer, (DWORD)size, &actual, nullptr);
            if (read_bytes)
            {
                *read_bytes = actual;
            }
            if (s)
            {
                return Result::success;
            }
            DWORD err = ::GetLastError();
            return translate_last_error(err);
        }
        Result write_unbuffered_file(HANDLE file, const void* buffer, usize size, usize* write_bytes)
        {
            luassert(file);
            DWORD actual;
            BOOL s = ::WriteFile(file, buffer, (DWORD)size, &actual, nullptr);
            if (write_bytes)
            {
                *write_bytes = actual;
            }
            if (s)
            {
                return Result::success;
            }
            DWORD err = ::GetLastError();
            return translate_last_error(err);
        }
        u64 get_unbuffered_file_size(HANDLE file)
        {
            luassert(file);
            LARGE_INTEGER size;
            if (::GetFileSizeEx(file, &size))
            {
                return size.QuadPart;
            }
            return 0;
        }
        Result set_unbuffered_file_size(HANDLE file, u64 sz)
        {
            luassert(file);
            LARGE_INTEGER old_cursor;
            LARGE_INTEGER cursor;
            LARGE_INTEGER end;
            end.QuadPart = (LONGLONG)sz;
            if (!::GetFileSizeEx(file, &old_cursor))
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            if (!::SetFilePointerEx(file, end, &cursor, FILE_BEGIN))
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            if (!::SetEndOfFile(file))
            {
                DWORD err = ::GetLastError();
                ::SetFilePointerEx(file, old_cursor, &cursor, FILE_BEGIN);
                return translate_last_error(err);
            }
            if (!::SetFilePointerEx(file, old_cursor, &cursor, FILE_BEGIN))
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        Result get_unbuffered_file_cursor(HANDLE file, u64& out_cursor)
        {
            luassert(file);
            LARGE_INTEGER cursor;
            LARGE_INTEGER movement;
            movement.QuadPart = 0;
            if (::SetFilePointerEx(file, movement, &cursor, FILE_CURRENT) == 0)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            out_cursor = cursor.QuadPart;
            return Result::success;
        }
        Result set_unbuffered_file_cursor(HANDLE file, i64 offset, SeekMode mode)
        {
            luassert(file != INVALID_HANDLE_VALUE);
            LARGE_INTEGER cursor;
            LARGE_INTEGER movement;
            movement.QuadPart = offset;
            DWORD method;
            switch (mode)
            {
            case SeekMode::begin:
                method = FILE_BEGIN;
                break;
            case SeekMode::current:
                method = FILE_CURRENT;
                break;
            case SeekMode::end:
                method = FILE_END;
                break;
            }
            if (::SetFilePointerEx(file, movement, &cursor, method) == 0)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        void flush_unbuffered_file(HANDLE file)
        {
            luassert(file);
            ::FlushFileBuffers(file);
        }
        Result open_buffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, FILE*& out_file)
        {
            lucheck(path);

            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            const wchar_t* mode;
            FILE* f = NULL;
            errno_t err;
            if (((flags & FileOpenFlag::read) != FileOpenFlag::none) && ((flags & FileOpenFlag::write) != FileOpenFlag::none))
            {
                // update mode.
                FileAttribute attr;
                switch (creation)
                {
                case FileCreationMode::create_always:
                    mode = L"w+b";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                case FileCreationMode::create_new:
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        memfree(pathbuffer);
                        return Result::already_exists;
                    }
                    mode = L"w+b";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                case FileCreationMode::open_always:
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        mode = L"r+b";
                        err = _wfopen_s(&f, pathbuffer, mode);
                    }
                    else
                    {
                        mode = L"w+b";
                        err = _wfopen_s(&f, pathbuffer, mode);
                    }
                    break;
                case FileCreationMode::open_existing:
                    mode = L"r+b";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                case FileCreationMode::open_existing_as_new:
                    if ((get_file_attribute(path, attr) == Result::success))
                    {
                        mode = L"w+b";
                        err = _wfopen_s(&f, pathbuffer, mode);
                    }
                    else
                    {
                        memfree(pathbuffer);
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
                    memfree(pathbuffer);
                    return Result::not_supported;    // Creates a new empty file and read-only from it has no meaning.
                    break;
                case FileCreationMode::open_existing:
                    mode = L"rb";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                default:
                    lupanic();
                    break;
                }
            }
            else if (((flags & FileOpenFlag::write) != FileOpenFlag::none))
            {
                // write only mode.
                FileAttribute attr;
                switch (creation)
                {
                case FileCreationMode::create_always:
                    mode = L"wb";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                case FileCreationMode::create_new:
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        memfree(pathbuffer);
                        return Result::already_exists;
                    }
                    mode = L"wb";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                case FileCreationMode::open_always:
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        mode = L"r+b";
                        err = _wfopen_s(&f, pathbuffer, mode);
                    }
                    else
                    {
                        mode = L"wb";
                        err = _wfopen_s(&f, pathbuffer, mode);
                    }
                    break;
                case FileCreationMode::open_existing:
                    mode = L"r+b";
                    err = _wfopen_s(&f, pathbuffer, mode);
                    break;
                case FileCreationMode::open_existing_as_new:
                    if (get_file_attribute(path, attr) == Result::success)
                    {
                        mode = L"wb";
                        err = _wfopen_s(&f, pathbuffer, mode);
                    }
                    else
                    {
                        memfree(pathbuffer);
                        return Result::not_found;
                    }
                    break;
                default:
                    lupanic();
                    break;
                }
            }
            memfree(pathbuffer);
            if (!f || err)
            {
                switch (err)
                {
                case EPERM:
                    return Result::access_denied;
                case ENOENT:
                    return Result::not_found;
                default:
                    return Result::bad_platform_call;
                }
            }
            out_file = f;
            return Result::success;
        }
        void close_buffered_file(FILE* file)
        {
            fclose(file);
        }
        Result read_buffered_file(FILE* file, void* buffer, usize size, usize* read_bytes)
        {
            lucheck(file);
            usize sz = _fread_nolock(buffer, 1, size, file);
            if (read_bytes)
            {
                *read_bytes = sz;
            }
            if (sz != size)
            {
                if (feof(file))
                {
                    clearerr(file);
                    return Result::success;
                }
                clearerr(file);
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        Result write_buffered_file(FILE* file, const void* buffer, usize size, usize* write_bytes)
        {
            lucheck(file);
            usize sz = _fwrite_nolock(buffer, 1, size, file);
            if (write_bytes)
            {
                *write_bytes = sz;
            }
            if (sz != size)
            {
                clearerr(file);
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        u64 get_buffered_file_size(FILE* file)
        {
            lucheck(file);
            HANDLE h = (HANDLE)_get_osfhandle(_fileno(file));
            LARGE_INTEGER size;
            if (::GetFileSizeEx(h, &size))
            {
                return size.QuadPart;
            }
            return 0;
        }
        Result set_buffered_file_size(FILE* file, u64 sz)
        {
            lucheck(file);
            HANDLE h = (HANDLE)_get_osfhandle(_fileno(file));
            return set_unbuffered_file_size(h, sz);
        }
        Result get_buffered_file_cursor(FILE* file, u64& out_cursor)
        {
            lucheck(file);
            __int64 cur = _ftelli64_nolock(file);
            if (cur < 0)
            {
                clearerr(file);
                return Result::bad_platform_call;
            }
            out_cursor = cur;
            return Result::success;
        }
        Result set_buffered_file_cursor(FILE* file, i64 offset, SeekMode mode)
        {
            lucheck(file);
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
            default:
                lupanic();
                break;
            }
            if (_fseeki64_nolock(file, offset, origin))
            {
                clearerr(file);
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        void flush_buffered_file(FILE* file)
        {
            lucheck(file);
            if (_fflush_nolock(file))
            {
                clearerr(file);
            }
        }

        Result open_file(const c8* path, FileOpenFlag flags, FileCreationMode creation, File& out_file)
        {
            bool buffered = test_flags(flags, FileOpenFlag::user_buffering);
            if (buffered)
            {
                FILE* f;
                auto r = open_buffered_file(path, flags, creation, f);
                if (r != Result::success)
                {
                    return r;
                }
                out_file.m_handle = (opaque_t)f;
            }
            else
            {
                HANDLE f;
                auto r = open_unbuffered_file(path, flags, creation, f);
                if (r != Result::success)
                {
                    return r;
                }
                out_file.m_handle = (opaque_t)f;
            }
            out_file.m_buffered = buffered;
            return Result::success;
        }
        void close_file(File& file)
        {
            if (file.m_buffered) close_buffered_file((FILE*)file.m_handle);
            else close_unbuffered_file((HANDLE)file.m_handle);
            file.m_handle = nullptr;
        }
        Result read_file(File& file, void* buffer, usize size, usize* read_bytes)
        {
            return file.m_buffered ? read_buffered_file((FILE*)file.m_handle, buffer, size, read_bytes) :
                read_unbuffered_file((HANDLE)file.m_handle, buffer, size, read_bytes);
        }
        Result write_file(File& file, const void* buffer, usize size, usize* write_bytes)
        {
            return file.m_buffered ? write_buffered_file((FILE*)file.m_handle, buffer, size, write_bytes) :
                write_unbuffered_file((HANDLE)file.m_handle, buffer, size, write_bytes);
        }
        u64 get_file_size(File& file)
        {
            return file.m_buffered ? get_buffered_file_size((FILE*)file.m_handle) : get_unbuffered_file_size((HANDLE)file.m_handle);
        }
        Result set_file_size(File& file, u64 sz)
        {
            return file.m_buffered ? set_buffered_file_size((FILE*)file.m_handle, sz) : set_unbuffered_file_size((HANDLE)file.m_handle, sz);
        }
        Result get_file_cursor(File& file, u64& out_cursor)
        {
            return file.m_buffered ? get_buffered_file_cursor((FILE*)file.m_handle, out_cursor) : get_unbuffered_file_cursor((HANDLE)file.m_handle, out_cursor);
        }
        Result set_file_cursor(File& file, i64 offset, SeekMode mode)
        {
            return file.m_buffered ? set_buffered_file_cursor((FILE*)file.m_handle, offset, mode) : set_unbuffered_file_cursor((HANDLE)file.m_handle, offset, mode);
        }
        void flush_file(File& file)
        {
            if (file.m_buffered) flush_buffered_file((FILE*)file.m_handle);
            else flush_unbuffered_file((HANDLE)file.m_handle);
        }
        inline i64 file_time_to_timestamp(const FILETIME& filetime)
        {
            ULARGE_INTEGER  ui;
            ui.LowPart = filetime.dwLowDateTime;
            ui.HighPart = filetime.dwHighDateTime;

            return ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
        }
        Result get_file_attribute(const c8* path, FileAttribute& out_attribute)
        {
            lucheck(path);
            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            WIN32_FILE_ATTRIBUTE_DATA d;
            BOOL r = ::GetFileAttributesExW(pathbuffer, GetFileExInfoStandard, &d);
            memfree(pathbuffer);
            if (!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            out_attribute.attributes = FileAttributeFlag::none;
            if (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            {
                out_attribute.attributes |= FileAttributeFlag::hidden;
            }
            if (d.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                out_attribute.attributes |= FileAttributeFlag::read_only;
            }
            if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                out_attribute.attributes |= FileAttributeFlag::directory;
            }
            out_attribute.size = ((u64)d.nFileSizeHigh << 32) + (u64)d.nFileSizeLow;
            out_attribute.creation_time = file_time_to_timestamp(d.ftCreationTime);
            out_attribute.last_access_time = file_time_to_timestamp(d.ftLastAccessTime);
            out_attribute.last_write_time = file_time_to_timestamp(d.ftLastWriteTime);
            return Result::success;
        }
        Result copy_file(const c8* from_path, const c8* to_path)
        {
            lucheck(from_path && to_path);
            wchar_t* from_buffer = utf8_to_wchar_buffered(from_path);
            wchar_t* to_buffer = utf8_to_wchar_buffered(to_path);
            BOOL r = CopyFileW(from_buffer, to_buffer, TRUE);
            memfree(from_buffer);
            memfree(to_buffer);
            if(!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        Result move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags)
        {
            lucheck(from_path && to_path);
            wchar_t* from_buffer = utf8_to_wchar_buffered(from_path);
            wchar_t* to_buffer = utf8_to_wchar_buffered(to_path);
            DWORD native_flags = test_flags(flags, FileMoveFlag::no_copy) ? 0 : MOVEFILE_COPY_ALLOWED;
            if(test_flags(flags, FileMoveFlag::allow_overwrite)) native_flags |= MOVEFILE_REPLACE_EXISTING;
            BOOL result = ::MoveFileExW(from_buffer, to_buffer, native_flags);
            DWORD error = result ? ERROR_SUCCESS : ::GetLastError();
            memfree(from_buffer);
            memfree(to_buffer);
            if(error == ERROR_NOT_SAME_DEVICE) return Result::not_supported;
            return translate_last_error(error);
        }
        static Result delete_single_file(const c8* path)
        {
            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            BOOL r = DeleteFileW(pathbuffer);
            memfree(pathbuffer);
            if(!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        static Result delete_empty_directory(const c8* path)
        {
            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            BOOL r = RemoveDirectoryW(pathbuffer);
            memfree(pathbuffer);
            if(!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        Result delete_file(const c8* path)
        {
            lucheck(path);
            FileAttribute attr;
            Result r = get_file_attribute(path, attr);
            if(r != Result::success) return r;
            if(test_flags(attr.attributes, FileAttributeFlag::directory))
            {
                // Delete empty directory.
                r = delete_empty_directory(path);
                if(r != Result::success) return r;
            }
            else
            {
                r = delete_single_file(path);
                if(r != Result::success) return r;
            }
            return Result::success;
        }
        Result open_dir(const c8* path, FileIterator& out_dir_iter)
        {
            usize buffer_size = utf8_to_utf16_len(path) + 3;    // for possible "/*" and null terminator.
            wchar_t* pathbuffer = (wchar_t*)memalloc(sizeof(wchar_t) * buffer_size);
            utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);
            // Append "\\*"
            if (pathbuffer[buffer_size - 4] == '/' || pathbuffer[buffer_size - 4] == '\\')
            {
                pathbuffer[buffer_size - 3] = '*';
                pathbuffer[buffer_size - 2] = 0;
            }
            else
            {
                pathbuffer[buffer_size - 3] = '/';
                pathbuffer[buffer_size - 2] = '*';
                pathbuffer[buffer_size - 1] = 0;
            }
            memzero(&out_dir_iter.m_data);
            out_dir_iter.m_allocated = true;
            out_dir_iter.m_file_name[0] = 0;
            out_dir_iter.m_h = ::FindFirstFileW(pathbuffer, &(out_dir_iter.m_data));
            memfree(pathbuffer);
            if (out_dir_iter.m_h == INVALID_HANDLE_VALUE)
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_FILE_NOT_FOUND)
                {
                    return Result::not_found;
                }
                else
                {
                    return Result::bad_platform_call;
                }
            }
            utf16_to_utf8(out_dir_iter.m_file_name, 512, (char16_t*)out_dir_iter.m_data.cFileName);
            return Result::success;
        }
        void close_dir(FileIterator& dir_iter)
        {
            if (dir_iter.m_h != INVALID_HANDLE_VALUE)
            {
                ::FindClose(dir_iter.m_h);
                dir_iter.m_h = INVALID_HANDLE_VALUE;
            }
        }
        bool dir_iterator_is_valid(FileIterator& dir_iter)
        {
            return dir_iter.m_allocated;
        }
        const c8* dir_iterator_get_filename(FileIterator& dir_iter)
        {
            if (dir_iter.m_allocated)
            {
                return dir_iter.m_file_name;
            }
            else
            {
                return nullptr;
            }
        }
        FileAttributeFlag dir_iterator_get_attributes(FileIterator& dir_iter)
        {
            if (!dir_iterator_is_valid(dir_iter))
            {
                return FileAttributeFlag::none;
            }
            DWORD attrs = dir_iter.m_data.dwFileAttributes;
            FileAttributeFlag r = FileAttributeFlag::none;
            if (attrs & FILE_ATTRIBUTE_HIDDEN)
            {
                r |= FileAttributeFlag::hidden;
            }
            if (attrs & FILE_ATTRIBUTE_READONLY)
            {
                r |= FileAttributeFlag::read_only;
            }
            if (attrs & FILE_ATTRIBUTE_DIRECTORY)
            {
                r |= FileAttributeFlag::directory;
            }
            return r;
        }
        bool dir_iterator_move_next(FileIterator& dir_iter)
        {
            if (!dir_iterator_is_valid(dir_iter))
            {
                return false;
            }
            if (::FindNextFileW(dir_iter.m_h, &dir_iter.m_data) == 0)
            {
                dir_iter.m_allocated = false;
                return false;
            }
            utf16_to_utf8(dir_iter.m_file_name, 512, (char16_t*)dir_iter.m_data.cFileName);
            dir_iter.m_allocated = true;
            return true;
        }
        Result create_dir(const c8* path)
        {
            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            BOOL r = ::CreateDirectoryW(pathbuffer, 0);
            memfree(pathbuffer);
            if (!r)
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_ALREADY_EXISTS)
                {
                    return Result::already_exists;
                }
                if (err == ERROR_PATH_NOT_FOUND)
                {
                    return Result::not_found;
                }
                return Result::bad_platform_call;
            }
            return Result::success;
        }
        Result remove_dir(const c8* path)
        {
            wchar_t* pathbuffer = utf8_to_wchar_buffered(path);
            BOOL r = ::RemoveDirectoryW(pathbuffer);
            memfree(pathbuffer);
            if (!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        const c8* get_current_dir()
        {
            DWORD sz = ::GetCurrentDirectoryW(0, NULL);
            wchar_t* path = (wchar_t*)memalloc(sizeof(wchar_t) * sz);
            ::GetCurrentDirectoryW(sz, path);
            usize len = utf16_to_utf8_len((char16_t*)path) + 1;
            c8* ret = (c8*)memalloc(sizeof(c8) * len);
            utf16_to_utf8(ret, len, (char16_t*)path);
            memfree(path);
            return ret;
        }
        void release_current_dir(const c8* dir)
        {
            memfree((c8*)dir);
        }
        Result set_current_dir(const c8* path)
        {
            wchar_t* dpath = utf8_to_wchar_buffered(path);
            BOOL r = ::SetCurrentDirectoryW(dpath);
            memfree(dpath);
            if(!r)
            {
                DWORD err = ::GetLastError();
                return translate_last_error(err);
            }
            return Result::success;
        }
        const c8* get_process_path()
        {
            DWORD buf_size = MAX_PATH;
            wchar_t* buf = (wchar_t*)memalloc(sizeof(wchar_t) * buf_size);
            DWORD sz = ::GetModuleFileNameW(NULL, buf, buf_size);
            while(sz == buf_size && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                buf_size += MAX_PATH;
                buf = (wchar_t*)memrealloc(buf, sizeof(wchar_t) * buf_size);
                sz = ::GetModuleFileNameW(NULL, buf, buf_size);
            }
            if(sz == 0)
            {
                lupanic_msg("GetModuleFileNameW failed");
            }
            usize len = utf16_to_utf8_len((char16_t*)buf) + 1;
            c8* ret = (c8*)memalloc(sizeof(c8) * len);
            utf16_to_utf8(ret, len, (char16_t*)buf);
            memfree(buf);
            return ret;
        }
        void release_process_path(const c8* path)
        {
            memfree((c8*)path);
        }
    }
}
