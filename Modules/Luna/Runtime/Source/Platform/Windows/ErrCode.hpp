/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ErrCode.hpp
* @author JXMaster
* @date 2023/3/26
*/
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../Result.hpp"

namespace Luna
{
    namespace Platform
    {
        inline Result translate_last_error(DWORD code)
        {
            switch(code)
            {
                case NO_ERROR: return Result::success;
                case ERROR_FILE_NOT_FOUND: 
                case ERROR_PATH_NOT_FOUND: return Result::not_found;
                case ERROR_TOO_MANY_OPEN_FILES: return Result::out_of_resource;
                case ERROR_ACCESS_DENIED: return Result::access_denied;
                case ERROR_INVALID_HANDLE: return Result::bad_arguments;
                case ERROR_NOT_ENOUGH_MEMORY: 
                case ERROR_OUTOFMEMORY: return Result::out_of_memory;
                case ERROR_INVALID_DATA: 
                case ERROR_CRC: return Result::bad_data;
                case ERROR_NOT_READY: return Result::not_ready;
                default: return Result::bad_platform_call;
            }
        }
    }
}