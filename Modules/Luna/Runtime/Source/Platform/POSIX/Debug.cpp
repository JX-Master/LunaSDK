/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.cpp
* @author JXMaster
* @date 2023/11/24
 */
#include "../../OS.hpp"
#include <execinfo.h>

namespace Luna
{
    namespace OS
    {
        u32 stack_backtrace(Span<opaque_t> frames)
        {
            return (u32)backtrace(frames.data(), (int)frames.size());
        }
        const c8** stack_backtrace_symbols(Span<const opaque_t> frames)
        {
            return (const c8**)backtrace_symbols(frames.data(), (int)frames.size());
        }
        void free_backtrace_symbols(const c8** symbols)
        {
            free(symbols);
        }
    }
}