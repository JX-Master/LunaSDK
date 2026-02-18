/*!
* This file is a portion of LunaSDK.
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
#include "Platform/DLL.hpp"
#include "Error.hpp"

namespace Luna
{
    LUNA_RUNTIME_API R<opaque_t> load_library(const c8* path)
    {
        opaque_t handle = nullptr;
        auto r = Platform::load_library(path, handle);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        return handle;
    }
    LUNA_RUNTIME_API void free_library(opaque_t handle)
    {
        Platform::free_library(handle);
    }
    LUNA_RUNTIME_API R<void*> get_library_function(opaque_t handle, const c8* symbol)
    {
        void* addr = nullptr;
        auto r = Platform::get_library_function(handle, symbol, addr);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        return addr;
    }
}