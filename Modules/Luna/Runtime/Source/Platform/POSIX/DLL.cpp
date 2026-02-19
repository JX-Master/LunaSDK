/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DLL.cpp
* @author JXMaster
* @date 2023/11/26
*/
#include "../DLL.hpp"
#include <dlfcn.h>
#include "../../../Error.hpp"

namespace Luna
{
    namespace Platform
    {
        Result load_library(const c8* path, opaque_t& out_handle)
        {
            void* h = dlopen(path, RTLD_LAZY);
            if(!h)
            {
                return Result::bad_platform_call;
            }
            out_handle = h;
            return Result::success;
        }
        void free_library(opaque_t handle)
        {
            dlclose((void*)handle);
        }
        Result get_library_function(opaque_t handle, const c8* symbol, void*& out_addr)
        {
            dlerror(); // clear old error.
            void* proc = dlsym((void*)handle, symbol);
            auto err = dlerror();
            if(err)
            {
                return Result::bad_platform_call;
            }
            out_addr = proc;
            return Result::success;
        }
    }
}