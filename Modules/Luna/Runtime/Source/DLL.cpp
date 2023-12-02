/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DLL.cpp
* @author JXMaster
* @date 2023/11/26
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../DLL.hpp"
#include "OS.hpp"

namespace Luna
{
    LUNA_RUNTIME_API R<opaque_t> load_library(const c8* path)
    {
        return OS::load_library(path);
    }
    LUNA_RUNTIME_API void free_library(opaque_t handle)
    {
        OS::free_library(handle);
    }
    LUNA_RUNTIME_API R<void*> get_library_function(opaque_t handle, const c8* symbol)
    {
        return OS::get_library_function(handle, symbol);
    }
}