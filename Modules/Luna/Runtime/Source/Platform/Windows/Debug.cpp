/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.cpp
* @author JXMaster
* @date 2023/11/24
*/
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../OS.hpp"
#include "../../../SpinLock.hpp"
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

namespace Luna
{
    namespace OS
    {
        HANDLE g_debug_process;
        SpinLock g_stack_backtrace_lock;
        void debug_init()
        {
            g_debug_process = GetCurrentProcess();
            SymInitialize(g_debug_process, NULL, TRUE);
        }
        void debug_close()
        {
            SymCleanup(g_debug_process);
        }
        u32 stack_backtrace(Span<opaque_t> frames)
        {
            LockGuard guard(g_stack_backtrace_lock);
            return CaptureStackBackTrace(3, (u32)frames.size(), frames.data(), NULL);
        }
        const c8** stack_backtrace_symbols(Span<const opaque_t> frames)
        {
            LockGuard guard(g_stack_backtrace_lock);
            usize ret_size = sizeof(const c8*) * frames.size();
            for(usize i = 0; i < frames.size(); ++i)
            {
                DWORD64 address = (DWORD64)(frames[i]);
                c8 buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
                PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
                memzero(symbol);
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                symbol->MaxNameLen = MAX_SYM_NAME;
                DWORD64 displacement = 0;
                if (SymFromAddr(g_debug_process, address, &displacement, symbol))
                {
                    DWORD line_displacement = 0;
                    IMAGEHLP_LINE64 line_info;
                    line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                    int len;
                    if(SymGetLineFromAddr64(g_debug_process, address, &line_displacement, &line_info))
                    {
                        len = snprintf(nullptr, 0, "0x%016llx %s [%s:%u]", (u64)symbol->Address, symbol->Name, line_info.FileName, line_info.LineNumber);
                    }
                    else
                    {
                        len = snprintf(nullptr, 0, "0x%016llx %s", (u64)symbol->Address, symbol->Name);
                    }
                    ret_size += (len + 1);
                }
            }
            c8** ret = (c8**)memalloc(ret_size);
            c8* str = (c8*)(ret + frames.size());
            usize str_size = ret_size - sizeof(const c8*) * frames.size();
            for(usize i = 0; i < frames.size(); ++i)
            {
                DWORD64 address = (DWORD64)(frames[i]);
                c8 buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
                PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
                memzero(symbol);
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                symbol->MaxNameLen = MAX_SYM_NAME;
                DWORD64 displacement = 0;
                if (SymFromAddr(g_debug_process, address, &displacement, symbol))
                {
                    DWORD line_displacement = 0;
                    IMAGEHLP_LINE64 line_info;
                    line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                    int len;
                    if(SymGetLineFromAddr64(g_debug_process, address, &line_displacement, &line_info))
                    {
                        len = snprintf(str, str_size, "0x%016llx %s [%s:%u]", (u64)symbol->Address, symbol->Name, line_info.FileName, line_info.LineNumber);
                    }
                    else
                    {
                        len = snprintf(str, str_size, "0x%016llx %s", (u64)symbol->Address, symbol->Name);
                    }
                    ret[i] = str;
                    str += (len + 1);
                    str_size -= (len + 1);
                }
                else
                {
                    ret[i] = nullptr;
                }
            }
            return (const c8**)ret;
        }
        void free_backtrace_symbols(const c8** symbols)
        {
            memfree(symbols);
        }
    }
}