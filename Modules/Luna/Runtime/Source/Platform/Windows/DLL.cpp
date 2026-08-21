/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DLL.cpp
* @author JXMaster
* @date 2023/11/26
*/
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../DLL.hpp"
#include "ErrCode.hpp"

namespace Luna
{
    namespace Platform
    {
        Result load_library(const c8* path, opaque_t& out_handle)
        {
            HMODULE h = LoadLibraryA(path);
            if(!h)
            {
                DWORD err = GetLastError();
                return translate_last_error(err);
            }
            out_handle = h;
            return Result::success;
        }
        void free_library(opaque_t handle)
        {
            FreeLibrary((HMODULE)handle);
        }
        Result get_library_function(opaque_t handle, const c8* symbol, void*& out_addr)
        {
            FARPROC proc = GetProcAddress((HMODULE)handle, symbol);
            if(!proc)
            {
                DWORD err = GetLastError();
                return translate_last_error(err);
            }
            out_addr = (void*)proc;
            return Result::success;
        }
    }
}
