/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.cpp
* @author JXMaster
* @date 2023/11/24
 */
#include "../../OS.hpp"
#if defined(LUNA_PLATFORM_ANDROID)
#include <unwind.h>
#include <stdlib.h>
#include <dlfcn.h>
#else
#include <execinfo.h>
#endif

namespace Luna
{
    namespace OS
    {
#if defined(LUNA_PLATFORM_ANDROID)
        struct BacktraceState
        {
            opaque_t* current;
            opaque_t* end;
        };
        _Unwind_Reason_Code unwind_callback( _Unwind_Context* context, void* arg)
        {
            BacktraceState* state = reinterpret_cast<BacktraceState*>(arg);
            if(state->current == state->end) return _URC_END_OF_STACK;
            uintptr_t pc = _Unwind_GetIP(context);
            if(pc)
            {
                *state->current = reinterpret_cast<opaque_t>(pc);
                ++state->current;
            }
            return _URC_NO_REASON;
        };
#endif
        u32 stack_backtrace(Span<opaque_t> frames)
        {
#if defined(LUNA_PLATFORM_ANDROID)
            BacktraceState state { frames.data(), frames.data() + frames.size() };
            _Unwind_Backtrace(unwind_callback, &state);
            return (u32)(state.current - frames.data());
#else
            return (u32)backtrace(frames.data(), (int)frames.size());
#endif
        }
        const c8** stack_backtrace_symbols(Span<const opaque_t> frames)
        {
#if defined(LUNA_PLATFORM_ANDROID)
            Vector<c8> symbols;
            String buf;
            for(usize i = 0; i < frames.size(); ++i)
            {
                buf.clear();
                Dl_info info;
                int r = dladdr(frames[i], &info);
                if(r)
                {
                    info.dli_fname = info.dli_fname ? info.dli_fname : "Unknown library";
                    info.dli_sname = info.dli_sname ? info.dli_sname : "Unknwon symbol";
                }
                else
                {
                    info.dli_fname = "Unknown library";
                    info.dli_sname = "Unknwon symbol";
                }
                strprintf(buf, "# %03u : 0x%016llx : %s(%s)", (u32)i, (u64)frames[i], info.dli_sname, info.dli_fname);
                symbols.insert(symbols.end(), buf.cspan());
                symbols.push_back('\0');
            }
            usize alloc_size = sizeof(c8*) * frames.size() + sizeof(c8) * buf.size();
            void* mem = malloc(alloc_size);
            const c8** arr = (const c8**)mem;
            c8* str = (c8*)((usize)mem + sizeof(c8*) * frames.size());
            memcpy(str, symbols.data(), symbols.size() * sizeof(c8));
            for(usize i = 0; i < frames.size(); ++i)
            {
                arr[i] = (const c8*)str;
                str += strlen(str) + 1;
            }
            return (const c8**)mem;
#else
            return (const c8**)backtrace_symbols(frames.data(), (int)frames.size());
#endif
        }
        void free_backtrace_symbols(const c8** symbols)
        {
            free(symbols);
        }
    }
}