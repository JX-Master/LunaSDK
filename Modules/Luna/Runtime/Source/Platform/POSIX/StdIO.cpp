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
#include "../../../Unicode.hpp"
#include <cstdio>
#include <pthread.h>

namespace Luna
{
    namespace OS
    {
        c32 g_input_buffer = 0;
        pthread_mutex_t g_std_io_mtx;

        void std_io_init()
        {
            luassert_msg_always(pthread_mutex_init(&g_std_io_mtx, NULL) == 0, "pthread_mutex_init failed.");
        }

        void std_io_close()
        {
            pthread_mutex_destroy(&g_std_io_mtx);
        }

        RV std_input(c8* buffer, usize size, usize* read_bytes)
        {
            luassert_msg_always(pthread_mutex_lock(&g_std_io_mtx) == 0, "pthread_mutex_lock failed.");
            if (size == 0)
            {
                if (read_bytes) *read_bytes = 0;
                return ok;
            }
            c8* cur = buffer;
            if(g_input_buffer)
            {
                c8 buf[6];
                usize len = utf8_encode_char(buf, g_input_buffer);
                if(cur + len < buffer + size)
                {
                    memcpy(cur, buf, len);
                    cur += len;
                    g_input_buffer = 0;
                }
                else
                {
                    luassert_msg_always(pthread_mutex_unlock(&g_std_io_mtx) == 0, "pthread_mutex_unlock failed.");
                    if(read_bytes) *read_bytes = 0;
                    return ok;
                }
            }
            c8 ch[6];
            int input_ch = EOF;
            while(cur < buffer + size - 1)
            {
                input_ch = getchar();
                if (input_ch == '\n' || input_ch == EOF)
                {
                    break;
                }
                ch[0] = (c8)input_ch;
                usize len = utf8_charlen(ch[0]);
                for(usize i = 1; i < len; ++i)
                {
                    input_ch = getchar();
                    if (input_ch == '\n' || input_ch == EOF) break;
                    ch[i] = (c8)input_ch;
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
            luassert_msg_always(pthread_mutex_unlock(&g_std_io_mtx) == 0, "pthread_mutex_unlock failed.");
            *cur = 0;
            if(read_bytes) *read_bytes = cur - buffer;
            if(input_ch == EOF) return feof(stdin) ? ok : BasicError::bad_platform_call();
            return ok;
        }

        RV std_output(const c8* buffer, usize size, usize* write_bytes)
        {
            luassert_msg_always(pthread_mutex_lock(&g_std_io_mtx) == 0, "pthread_mutex_lock failed.");
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
            luassert_msg_always(pthread_mutex_unlock(&g_std_io_mtx) == 0, "pthread_mutex_unlock failed.");
            if(write_bytes) *write_bytes = cur - buffer;
            return ok;
        }
    }
}