/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StdIO.cpp
* @author JXMaster
* @date 2023/2/28
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../../Unicode.hpp"

namespace Luna
{
    namespace OS
    {
        c32 g_input_buffer = 0;
        CRITICAL_SECTION g_std_io_mtx;

        UINT g_oldcp;
        UINT g_old_outputcp;

        void std_io_init()
        {
            InitializeCriticalSection(&g_std_io_mtx);
            g_oldcp = GetConsoleCP();
            g_old_outputcp = GetConsoleOutputCP();
            SetConsoleCP(CP_UTF8);
            SetConsoleOutputCP(CP_UTF8);
        }

        void std_io_close()
        {
            SetConsoleCP(g_oldcp);
            SetConsoleOutputCP(g_old_outputcp);
            DeleteCriticalSection(&g_std_io_mtx);
        }

        RV std_input(c8* buffer, usize size, usize* read_bytes)
        {
            EnterCriticalSection(&g_std_io_mtx);
            c8* cur = buffer;
            if(g_input_buffer)
            {
                c8 buf[6];
                usize len = utf8_encode_char(buf, g_input_buffer);
                if(cur + len <= buffer + size)
                {
                    memcpy(cur, buf, len);
                    cur += len;
                    g_input_buffer = 0;
                }
                else
                {
                    LeaveCriticalSection(&g_std_io_mtx);
                    if(read_bytes) *read_bytes = 0;
                    return ok;
                }
            }
            c8 ch[6];
            while(cur < buffer + size - 1)
            {
                ch[0] = getchar();
                if(ch[0] == '\n' || ch[0] == EOF)
                {
                    break;
                }
                usize len = utf8_charlen(ch[0]);
                for(usize i = 1; i < len; ++i)
                {
                    ch[i] = getchar();
                }
                // Encode this character.
                if(cur + len < buffer + size)
                {
                    memcpy(cur, ch, len);
                    cur += len;
                }
                else
                {
                    g_input_buffer = utf8_decode_char(ch);
                    break;
                }
            }
            LeaveCriticalSection(&g_std_io_mtx);
            *cur = 0;
            if(read_bytes) *read_bytes = cur - buffer;
            if(ch[0] == EOF) return feof(stdin) ? ok : BasicError::bad_platform_call();
            return ok;
        }

        RV std_output(const c8* buffer, usize size, usize* write_bytes)
        {
            EnterCriticalSection(&g_std_io_mtx);
            const c8* cur = buffer;
            while(cur < buffer + size)
            {
                if(*cur == '\0') break;
                usize len = utf8_charlen(*cur);
                if(cur + len > buffer + size) break;
                for(usize i = 0; i < len; ++i)
                {
                    putchar(cur[i]);
                }
                cur += len;
            }
            LeaveCriticalSection(&g_std_io_mtx);
            if(write_bytes) *write_bytes = cur - buffer;
            return ok;
        }
    }
}