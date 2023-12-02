/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ErrCode.hpp
* @author JXMaster
* @date 2023/3/26
*/
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../../Result.hpp"

namespace Luna
{
    namespace OS
    {
        inline ErrCode translate_last_error(DWORD code)
        {
            switch(code)
            {
                case NO_ERROR: return ErrCode(0);
                case ERROR_FILE_NOT_FOUND: 
                case ERROR_PATH_NOT_FOUND: return BasicError::not_found();
                case ERROR_TOO_MANY_OPEN_FILES: return BasicError::out_of_resource();
                case ERROR_ACCESS_DENIED: return BasicError::access_denied();
                case ERROR_INVALID_HANDLE: return BasicError::bad_arguments();
                case ERROR_NOT_ENOUGH_MEMORY: 
                case ERROR_OUTOFMEMORY: return BasicError::out_of_memory();
                case ERROR_INVALID_DATA: 
                case ERROR_CRC: return BasicError::bad_data();
                case ERROR_NOT_READY: return BasicError::not_ready();
                default: return BasicError::bad_platform_call();
            }
        }
    }
}