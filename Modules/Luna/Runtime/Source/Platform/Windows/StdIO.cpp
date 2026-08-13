/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.cpp
* @author JXMaster
* @date 2023/2/28
*/
#include "../StdIO.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"
#include "ErrCode.hpp"

namespace Luna
{
    namespace Platform
    {
        static HANDLE get_standard_handle(DWORD handle_id)
        {
            HANDLE handle = GetStdHandle(handle_id);
            return handle == INVALID_HANDLE_VALUE ? nullptr : handle;
        }

        static DWORD clamp_io_size(usize size)
        {
            return size > (usize)MAXDWORD ? MAXDWORD : (DWORD)size;
        }

        Result read_standard_input(void* buffer, usize size, usize* read_bytes)
        {
            if (read_bytes) *read_bytes = 0;
            if (!size) return Result::success;
            HANDLE handle = get_standard_handle(STD_INPUT_HANDLE);
            if (!handle) return Result::bad_file;
            DWORD actual_bytes = 0;
            if (!ReadFile(handle, buffer, clamp_io_size(size), &actual_bytes, nullptr))
            {
                DWORD error = GetLastError();
                if (error == ERROR_BROKEN_PIPE || error == ERROR_HANDLE_EOF) return Result::success;
                if (error == ERROR_INVALID_HANDLE) return Result::bad_file;
                return translate_last_error(error);
            }
            if (read_bytes) *read_bytes = (usize)actual_bytes;
            return Result::success;
        }

        static Result write_standard_stream(DWORD handle_id, const void* buffer, usize size, usize* write_bytes)
        {
            if (write_bytes) *write_bytes = 0;
            if (!size) return Result::success;
            HANDLE handle = get_standard_handle(handle_id);
            if (!handle) return Result::bad_file;
            DWORD actual_bytes = 0;
            if (!WriteFile(handle, buffer, clamp_io_size(size), &actual_bytes, nullptr))
            {
                DWORD error = GetLastError();
                if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA) return Result::bad_pipe;
                if (error == ERROR_INVALID_HANDLE) return Result::bad_file;
                return translate_last_error(error);
            }
            if (write_bytes) *write_bytes = (usize)actual_bytes;
            return Result::success;
        }

        Result write_standard_output(const void* buffer, usize size, usize* write_bytes)
        {
            return write_standard_stream(STD_OUTPUT_HANDLE, buffer, size, write_bytes);
        }

        Result write_standard_error(const void* buffer, usize size, usize* write_bytes)
        {
            return write_standard_stream(STD_ERROR_HANDLE, buffer, size, write_bytes);
        }
    }
}
