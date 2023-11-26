/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DLL.cpp
* @author JXMaster
* @date 2023/11/26
*/
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../OS.hpp"
#include "ErrCode.hpp"

namespace Luna
{
    namespace OS
    {
        R<opaque_t> load_library(const c8* path)
        {
            HMODULE h = LoadLibraryA(path);
            if(!h)
            {
                DWORD err = GetLastError();
                return translate_last_error(err);
            }
            return (opaque_t)h;
        }
        void free_library(opaque_t handle)
        {
            FreeLibrary((HMODULE)handle);
        }
        R<void*> get_library_function(opaque_t handle, const c8* symbol)
        {
            FARPROC proc = GetProcAddress((HMODULE)handle, symbol);
            if(!proc)
            {
                DWORD err = GetLastError();
                return translate_last_error(err);
            }
            return (void*)proc;
        }
    }
}