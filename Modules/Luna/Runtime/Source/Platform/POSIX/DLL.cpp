/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DLL.cpp
* @author JXMaster
* @date 2023/11/26
*/
#include "../../OS.hpp"
#include <dlfcn.h>
#include "../../../Error.hpp"

namespace Luna
{
    namespace OS
    {
        R<opaque_t> load_library(const c8* path)
        {
            void* h = dlopen(path, RTLD_LAZY);
            if(!h)
            {
                return set_error(BasicError::bad_platform_call(), "%s", dlerror());
            }
            return (opaque_t)h;
        }
        void free_library(opaque_t handle)
        {
            dlclose((void*)handle);
        }
        R<void*> get_library_function(opaque_t handle, const c8* symbol)
        {
            dlerror(); // clear old error.
            void* proc = dlsym((void*)handle, symbol);
            auto err = dlerror();
            if(err)
            {
                return set_error(BasicError::bad_platform_call(), "%s", dlerror());
            }
            return proc;
        }
    }
}