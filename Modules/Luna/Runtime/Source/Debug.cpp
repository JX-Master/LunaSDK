/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.cpp
* @author JXMaster
* @date 2023/11/24
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Debug.hpp"
#include "Platform/Debug.hpp"

namespace Luna
{
    LUNA_RUNTIME_API u32 stack_backtrace(Span<opaque_t> frames)
    {
        return Platform::stack_backtrace(frames);
    }
    LUNA_RUNTIME_API const c8** stack_backtrace_symbols(Span<const opaque_t> frames)
    {
        return Platform::stack_backtrace_symbols(frames);
    }
    LUNA_RUNTIME_API void free_backtrace_symbols(const c8** symbols)
    {
        Platform::free_backtrace_symbols(symbols);
    }
}