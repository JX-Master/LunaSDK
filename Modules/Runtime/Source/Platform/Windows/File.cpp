/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file File.cpp
* @author JXMaster
* @date 2019/9/29
*/
#include <Runtime/PlatformDefines.hpp>
#ifdef LUNA_PLATFORM_WINDOWS

#define LUNA_RUNTIME_API LUNA_EXPORT

#include "../../../Platform/Windows/MiniWin.hpp"
#include <io.h>
#include <Runtime/Unicode.hpp>
#include "../../OS.hpp"
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

namespace Luna
{
	namespace OS
	{
		R<opaque_t> open_unbuffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation)
		{
			lucheck(path);

			usize buffer_size = utf8_to_utf16_len(path) + 1;
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
			utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);

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
			HANDLE fileHandle = ::CreateFileW(pathbuffer, dw_access, FILE_SHARE_READ, nullptr, dw_creation, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				DWORD dw = ::GetLastError();
				if (dw == ERROR_ACCESS_DENIED)
				{
					return BasicError::access_denied();
				}
				else if (dw == ERROR_FILE_NOT_FOUND)
				{
					return BasicError::not_found();
				}
				else if (dw == ERROR_PATH_NOT_FOUND)
				{
					return BasicError::not_found();
				}
				else
				{
					return BasicError::bad_platform_call();
				}
			}
			return fileHandle;
		}
		void close_unbuffered_file(opaque_t file)
		{
			::CloseHandle((HANDLE)file);
		}
		RV read_unbuffered_file(opaque_t file, void* buffer, usize size, usize* read_bytes)
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
				return ok;
			}
			DWORD err = ::GetLastError();
			return BasicError::bad_platform_call();
		}
		RV write_unbuffered_file(opaque_t file, const void* buffer, usize size, usize* write_bytes)
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
				return ok;
			}
			DWORD err = ::GetLastError();
			return BasicError::bad_platform_call();
		}
		u64 get_unbuffered_file_size(opaque_t file)
		{
			luassert(file);
			LARGE_INTEGER size;
			if (::GetFileSizeEx(file, &size))
			{
				return size.QuadPart;
			}
			return 0;
		}
		RV set_unbuffered_file_size(opaque_t file, u64 sz)
		{
			luassert(file);
			LARGE_INTEGER old_cursor;
			LARGE_INTEGER cursor;
			LARGE_INTEGER end;
			end.QuadPart = (LONGLONG)sz;
			if (!::GetFileSizeEx(file, &old_cursor))
			{
				return BasicError::bad_platform_call();
			}
			if (!::SetFilePointerEx(file, end, &cursor, FILE_BEGIN))
			{
				return BasicError::bad_platform_call();
			}
			if (!::SetEndOfFile(file))
			{
				::SetFilePointerEx(file, old_cursor, &cursor, FILE_BEGIN);
				return BasicError::bad_platform_call();
			}
			if (!::SetFilePointerEx(file, old_cursor, &cursor, FILE_BEGIN))
			{
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		R<u64> get_unbuffered_file_cursor(opaque_t file)
		{
			luassert(file);
			LARGE_INTEGER cursor;
			LARGE_INTEGER movement;
			movement.QuadPart = 0;
			if (::SetFilePointerEx(file, movement, &cursor, FILE_CURRENT) == 0)
			{
				DWORD err = ::GetLastError();
				return R<u64>::failure(BasicError::bad_platform_call());
			}
			return cursor.QuadPart;
		}
		RV set_unbuffered_file_cursor(opaque_t file, i64 offset, SeekMode mode)
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
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		void flush_unbuffered_file(opaque_t file)
		{
			luassert(file);
			::FlushFileBuffers(file);
		}
		R<opaque_t> open_buffered_file(const c8* path, FileOpenFlag flags, FileCreationMode creation)
		{
			lucheck(path);

			usize buffer_size = utf8_to_utf16_len(path) + 1;
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
			utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);
			const wchar_t* mode;
			FILE* f = NULL;
			errno_t err;
			if (((flags & FileOpenFlag::read) != FileOpenFlag::none) && ((flags & FileOpenFlag::write) != FileOpenFlag::none))
			{
				// update mode.
				switch (creation)
				{
				case FileCreationMode::create_always:
					mode = L"w+b";
					err = _wfopen_s(&f, pathbuffer, mode);
					break;
				case FileCreationMode::create_new:
					if (file_attribute(path).valid())
					{
						return BasicError::already_exists;
					}
					mode = L"w+b";
					err = _wfopen_s(&f, pathbuffer, mode);
					break;
				case FileCreationMode::open_always:
					if (file_attribute(path).valid())
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
					if ((file_attribute(path).valid()))
					{
						mode = L"w+b";
						err = _wfopen_s(&f, pathbuffer, mode);
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
					return BasicError::not_supported();	// Creates a new empty file and read-only from it has no meaning.
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
				switch (creation)
				{
				case FileCreationMode::create_always:
					mode = L"wb";
					err = _wfopen_s(&f, pathbuffer, mode);
					break;
				case FileCreationMode::create_new:
					if (file_attribute(path).valid())
					{
						return BasicError::already_exists();
					}
					mode = L"wb";
					err = _wfopen_s(&f, pathbuffer, mode);
					break;
				case FileCreationMode::open_always:
					if (file_attribute(path).valid())
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
					if (file_attribute(path).valid())
					{
						mode = L"wb";
						err = _wfopen_s(&f, pathbuffer, mode);
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
			if (!f || err)
			{
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
			return f;
		}
		void close_buffered_file(opaque_t file)
		{
			fclose((FILE*)file);
		}
		RV read_buffered_file(opaque_t file, void* buffer, usize size, usize* read_bytes)
		{
			lucheck(file);
			usize sz = _fread_nolock(buffer, 1, size, (FILE*)file);
			if (read_bytes)
			{
				*read_bytes = sz;
			}
			if (sz != size)
			{
				if (feof((FILE*)file))
				{
					clearerr((FILE*)file);
					return ok;
				}
				clearerr((FILE*)file);
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		RV write_buffered_file(opaque_t file, const void* buffer, usize size, usize* write_bytes)
		{
			lucheck(file);
			usize sz = _fwrite_nolock(buffer, 1, size, (FILE*)file);
			if (write_bytes)
			{
				*write_bytes = sz;
			}
			if (sz != size)
			{
				clearerr((FILE*)file);
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		u64 get_buffered_file_size(opaque_t file)
		{
			lucheck(file);
			HANDLE h = (HANDLE)_get_osfhandle(_fileno((FILE*)file));
			LARGE_INTEGER size;
			if (::GetFileSizeEx(h, &size))
			{
				return size.QuadPart;
			}
			return 0;
		}
		RV set_buffered_file_size(opaque_t file, u64 sz)
		{
			lucheck(file);
			HANDLE h = (HANDLE)_get_osfhandle(_fileno((FILE*)file));
			return set_file_size(h, sz);
		}
		R<u64> get_buffered_file_cursor(opaque_t file)
		{
			lucheck(file);
			__int64 cur = _ftelli64_nolock((FILE*)file);
			if (cur < 0)
			{
				clearerr((FILE*)file);
				return R<u64>::failure(BasicError::bad_platform_call());
			}
			return (u64)cur;
		}
		RV set_buffered_file_cursor(opaque_t file, i64 offset, SeekMode mode)
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
			if (_fseeki64_nolock((FILE*)file, offset, origin))
			{
				clearerr((FILE*)file);
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		void flush_buffered_file(opaque_t file)
		{
			lucheck(file);
			if (_fflush_nolock((FILE*)file))
			{
				clearerr((FILE*)file);
			}
		}

		struct File
		{
			opaque_t handle;
			bool buffered;
		};
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
		RV read_file(opaque_t file, Span<byte_t> buffer, usize* read_bytes)
		{
			File* f = (File*)file;
			return f->buffered ? read_buffered_file(f->handle, buffer.data(), buffer.size(), read_bytes) :
				read_unbuffered_file(f->handle, buffer.data(), buffer.size(), read_bytes);
		}
		RV write_file(opaque_t file, Span<const byte_t> buffer, usize* write_bytes)
		{
			File* f = (File*)file;
			return f->buffered ? write_buffered_file(f->handle, buffer.data(), buffer.size(), write_bytes) :
				write_unbuffered_file(f->handle, buffer.data(), buffer.size(), write_bytes);
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
		inline i64 file_time_to_timestamp(const FILETIME& filetime)
		{
			ULARGE_INTEGER  ui;
			ui.LowPart = filetime.dwLowDateTime;
			ui.HighPart = filetime.dwHighDateTime;

			return ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
		}
		R<FileAttribute> file_attribute(const c8* path)
		{
			lucheck(path);
			usize buffer_size = utf8_to_utf16_len(path) + 1;
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
			utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);
			WIN32_FILE_ATTRIBUTE_DATA d;
			if (!::GetFileAttributesExW(pathbuffer, GetFileExInfoStandard, &d))
			{
				return BasicError::bad_platform_call();
			}
			FileAttribute attribute;
			attribute.attributes = FileAttributeFlag::none;
			if (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{
				attribute.attributes |= FileAttributeFlag::hidden;
			}
			if (d.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				attribute.attributes |= FileAttributeFlag::read_only;
			}
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				attribute.attributes |= FileAttributeFlag::directory;
			}
			attribute.size = ((u64)d.nFileSizeHigh << 32) + (u64)d.nFileSizeLow;
			attribute.creation_time = file_time_to_timestamp(d.ftCreationTime);
			attribute.last_access_time = file_time_to_timestamp(d.ftLastAccessTime);
			attribute.last_write_time = file_time_to_timestamp(d.ftLastWriteTime);
			return attribute;
		}
		RV copy_file(const c8* from_path, const c8* to_path, FileCopyFlag flags)
		{
			lucheck(from_path && to_path);
			usize from_size = utf8_to_utf16_len(from_path) + 2;
			usize to_size = utf8_to_utf16_len(to_path) + 2;
			wchar_t* fromBuffer = (wchar_t*)alloca(sizeof(wchar_t) * from_size);
			wchar_t* toBuffer = (wchar_t*)alloca(sizeof(wchar_t) * to_size);
			utf8_to_utf16((char16_t*)fromBuffer, from_size, from_path);
			utf8_to_utf16((char16_t*)toBuffer, to_size, to_path);
			fromBuffer[from_size - 1] = 0;
			toBuffer[to_size - 1] = 0;
			if (test_flags(flags, FileCopyFlag::fail_if_exists))
			{
				auto attr = file_attribute(to_path);
				if (succeeded(attr)) return BasicError::already_exists();
			}
			SHFILEOPSTRUCTW FileOp = { 0 };
			FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
			FileOp.pFrom = fromBuffer;
			FileOp.pTo = toBuffer;
			FileOp.wFunc = FO_COPY;
			int r = SHFileOperationW(&FileOp);
			return r ? BasicError::bad_platform_call() : ok;
		}
		RV move_file(const c8* from_path, const c8* to_path, FileMoveFlag flags)
		{
			lucheck(from_path && to_path);
			usize from_size = utf8_to_utf16_len(from_path) + 2;
			usize to_size = utf8_to_utf16_len(to_path) + 2;
			wchar_t* fromBuffer = (wchar_t*)alloca(sizeof(wchar_t) * from_size);
			wchar_t* toBuffer = (wchar_t*)alloca(sizeof(wchar_t) * to_size);
			utf8_to_utf16((char16_t*)fromBuffer, from_size, from_path);
			utf8_to_utf16((char16_t*)toBuffer, to_size, to_path);
			fromBuffer[from_size - 1] = 0;
			toBuffer[to_size - 1] = 0;
			if (test_flags(flags, FileMoveFlag::fail_if_exists))
			{
				auto attr = file_attribute(to_path);
				if (succeeded(attr)) return BasicError::already_exists();
			}
			SHFILEOPSTRUCTW FileOp = { 0 };
			FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
			FileOp.pFrom = fromBuffer;
			FileOp.pTo = toBuffer;
			FileOp.wFunc = FO_MOVE;
			int r = SHFileOperationW(&FileOp);
			return r ? BasicError::bad_platform_call() : ok;
		}
		RV delete_file(const c8* path, FileDeleteFlag flags)
		{
			lucheck(path);
			usize buffer_size = utf8_to_utf16_len(path) + 2;
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
			utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);
			pathbuffer[buffer_size - 1] = 0;
			auto attr = file_attribute(path);
			if (failed(attr)) return BasicError::not_found();
			SHFILEOPSTRUCTW FileOp = { 0 };
			FileOp.fFlags = FOF_NOCONFIRMATION;
			if (test_flags(flags, FileDeleteFlag::allow_undo))
			{
				FileOp.fFlags |= FOF_ALLOWUNDO;
			}
			FileOp.pFrom = pathbuffer;
			FileOp.pTo = NULL;
			FileOp.wFunc = FO_DELETE;
			int r = SHFileOperationW(&FileOp);
			return r ? BasicError::bad_platform_call() : ok;
		}
		struct FileData
		{
			WIN32_FIND_DATAW m_data;
			HANDLE m_h;
			const char* m_filename_ptr;
			char m_file_name[260];	// Buffer to store the file name in UTF-8 format.
			bool m_allocated;

			FileData() :
				m_allocated(true),
				m_h(INVALID_HANDLE_VALUE)
			{
				m_file_name[0] = 0;
				m_filename_ptr = m_file_name;
			}
			~FileData()
			{
				if (m_h != INVALID_HANDLE_VALUE)
				{
					::FindClose(m_h);
					m_h = INVALID_HANDLE_VALUE;
				}
			}
		};
		R<opaque_t> open_dir(const c8* path)
		{
			usize buffer_size = utf8_to_utf16_len(path) + 3;	// for possible "/*" and null terminator.
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
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
			FileData* data = Luna::memnew<FileData>();
			
			data->m_h = ::FindFirstFileW(pathbuffer, &(data->m_data));
			if (data->m_h == INVALID_HANDLE_VALUE)
			{
				Luna::memdelete(data);
				DWORD err = ::GetLastError();
				if (err == ERROR_FILE_NOT_FOUND)
				{
					return BasicError::not_found();
				}
				else
				{
					return BasicError::bad_platform_call();
				}
			}
			utf16_to_utf8(data->m_file_name, 260, (char16_t*)data->m_data.cFileName);
			return data;
		}
		void close_dir(opaque_t dir_iter)
		{
			Luna::memdelete((FileData*)dir_iter);
		}
		bool dir_iterator_valid(opaque_t dir_iter)
		{
			return ((FileData*)dir_iter)->m_allocated;
		}
		const c8* dir_iterator_filename(opaque_t dir_iter)
		{
			if (((FileData*)dir_iter)->m_allocated)
			{
				return ((FileData*)dir_iter)->m_filename_ptr;
			}
			else
			{
				return nullptr;
			}
		}
		FileAttributeFlag dir_iterator_attribute(opaque_t dir_iter)
		{
			FileData* f = (FileData*)dir_iter;
			if (!dir_iterator_valid(dir_iter))
			{
				return FileAttributeFlag::none;
			}
			DWORD attrs = f->m_data.dwFileAttributes;
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
		bool dir_iterator_move_next(opaque_t dir_iter)
		{
			FileData* f = (FileData*)dir_iter;
			if (!dir_iterator_valid(dir_iter))
			{
				return false;
			}
			if (::FindNextFileW(f->m_h, &f->m_data) == 0)
			{
				f->m_allocated = false;
				return false;
			}
			utf16_to_utf8(f->m_file_name, 260, (char16_t*)f->m_data.cFileName);
			f->m_allocated = true;
			return true;
		}
		RV	create_dir(const c8* path)
		{
			usize buffer_size = utf8_to_utf16_len(path) + 1;
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
			utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);
			BOOL r = ::CreateDirectoryW(pathbuffer, 0);
			if (!r)
			{
				DWORD err = ::GetLastError();
				if (err == ERROR_ALREADY_EXISTS)
				{
					return BasicError::already_exists();
				}
				if (err == ERROR_PATH_NOT_FOUND)
				{
					return BasicError::not_found();
				}
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		RV	remove_dir(const c8* path)
		{
			usize buffer_size = utf8_to_utf16_len(path) + 1;
			wchar_t* pathbuffer = (wchar_t*)alloca(sizeof(wchar_t) * buffer_size);
			utf8_to_utf16((char16_t*)pathbuffer, buffer_size, path);
			BOOL r = ::RemoveDirectoryW(pathbuffer);
			if (!r)
			{
				DWORD err = ::GetLastError();
				return BasicError::bad_platform_call();
			}
			return ok;
		}
		u32 get_current_dir(u32 buffer_length, c8* buffer)
		{
			DWORD sz = ::GetCurrentDirectoryW(0, NULL);
			wchar_t* path = (wchar_t*)alloca(sizeof(wchar_t) * sz);
			::GetCurrentDirectoryW(sz, path);
			if (buffer && buffer_length)
			{
				utf16_to_utf8(buffer, buffer_length, (char16_t*)path);
			}
			return (u32)utf16_to_utf8_len((char16_t*)path);
		}
		RV set_current_dir(const c8* path)
		{
			usize u16len = utf8_to_utf16_len(path);
			wchar_t* dpath = (wchar_t*)alloca(sizeof(wchar_t) * (u16len + 1));
			utf8_to_utf16((char16_t*)dpath, u16len + 1, path);
			if (FAILED(::SetCurrentDirectoryW(dpath)))
			{
				return BasicError::bad_platform_call();
			}
			return ok;
		}

		c8 g_process_path[1024];

		void file_init()
		{
			wchar_t pathbuffer[1024];
			DWORD size = ::GetModuleFileNameW(NULL, pathbuffer, 1024);
			utf16_to_utf8(g_process_path, 1024, (char16_t*)pathbuffer);
		}

		const c8* get_process_path()
		{
			return g_process_path;
		}
	}


}

#endif
