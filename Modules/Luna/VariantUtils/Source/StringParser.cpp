/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StringParser.cpp
* @author JXMaster
* @date 2023/8/28
*/
#include "StringParser.hpp"

namespace Luna
{
    namespace VariantUtils
    {
        void BufferReadContext::consume(c32 ch)
        {
            if (ch == 0) return;
            if(encoding == Encoding::utf_8)
            {
                const c8* cur_utf8 = (const c8*)cur;
                cur_utf8 += utf8_charspan(ch);
                cur = cur_utf8;
            }
            else if(encoding == Encoding::utf_16_le || encoding == Encoding::utf_16_be)
            {
                const c16* cur_utf16 = (const c16*)cur;
                cur_utf16 += utf16_charspan(ch);
                cur = cur_utf16;
            }
            else
            {
                lupanic();
            }
            if (ch == '\n')
            {
                pos = 1;
                ++line;
            }
            else
            {
                ++pos;
            }
        }
        inline c16 utf16_read_char(c16 c, Encoding encoding)
        {
            u16 ch = (u16)c;
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
            if(encoding == Encoding::utf_16_be)
#else
            if(encoding == Encoding::utf_16_le)
#endif
            {
                ch = (ch << 8) | (ch >> 8);
            }
            return (c16)ch;
        }
        inline c32 utf16_decode_char_encoding(const c16* string, Encoding encoding)
        {
            c16 buf[2];
            usize charlen;
            buf[0] = utf16_read_char(*string, encoding);
            charlen = utf16_charlen(buf[0]);
            if(charlen > 1)
            {
                buf[1] = utf16_read_char(*(string + 1), encoding);
            }
            return utf16_decode_char(buf);
        }
        R<c32> BufferReadContext::next_char(usize index)
        {
            if(encoding == Encoding::utf_8)
            {
                const c8* cur_utf8 = (const c8*)cur;
                const c8* next_cur = cur_utf8;
                const c8* end = (const c8*)src + src_size;
                while(index)
                {
                    // advance characters.
                    if(next_cur >= end) return (c32)0;
                    usize num_bytes;
                    R<c32> ch = utf8_decode_char(next_cur, (usize)(end - next_cur), &num_bytes);
                    if(failed(ch)) return ch.errcode();
                    if(!ch.get()) return BasicError::bad_data();
                    next_cur += num_bytes;
                    --index;
                }
                if(next_cur >= end) return (c32)0;
                R<c32> ch = utf8_decode_char(next_cur, (usize)(end - next_cur));
                if(failed(ch)) return ch.errcode();
                if(!ch.get()) return BasicError::bad_data();
                return ch.get();
            }
            else if(encoding == Encoding::utf_16_le || encoding == Encoding::utf_16_be)
            {
                const c16* cur_utf16 = (const c16*)cur;
                const c16* next_cur = cur_utf16;
                while(index)
                {
                    // advance characters.
                    c32 ch = utf16_decode_char_encoding(next_cur, encoding);
                    if (!ch) return BasicError::bad_data();
                    next_cur += utf16_charspan(ch);
                    if ((usize)(next_cur - (const c16*)src) * 2 >= src_size) return (c32)0;
                    --index;
                }
                if ((usize)(next_cur - (const c16*)src) * 2 >= src_size) return (c32)0;
                c32 ch = utf16_decode_char_encoding(next_cur, encoding);
                if(!ch) return BasicError::bad_data();
                return ch;
            }
            lupanic();
            return BasicError::bad_data();
        }
        void BufferReadContext::skip_utf16_bom()
        {
            if(src_size >= 2)
            {
                const u8* ch = (const u8*)src;
                if(ch[0] == 0xFE && ch[1] == 0xFF)
                {
                    encoding = Encoding::utf_16_be;
                    src = ch + 2;
                    cur = ch + 2;
                    src_size -= 2;
                }
                else if(ch[0] == 0xFF && ch[1] == 0xFE)
                {
                    encoding = Encoding::utf_16_le;
                    src = ch + 2;
                    cur = ch + 2;
                    src_size -= 2;
                }
            }
        }
        void StreamReadContext::consume(c32 ch)
        {
            if (ch == 0) return;
            buffer.pop_front();
            if (ch == '\n')
            {
                pos = 1;
                ++line;
            }
            else
            {
                ++pos;
            }
        }
        RV StreamReadContext::stream_read(void* buf, usize read_size, usize* read_bytes)
        {
            usize real_read_bytes = 0;
            u8* buf_u8 = (u8*)buf;
            while(read_size && !stream_buffer.empty())
            {
                *buf_u8 = stream_buffer.front();
                stream_buffer.pop_front();
                ++buf_u8;
                ++real_read_bytes;
                --read_size;
            }
            lutry
            {
                if(read_size)
                {
                    usize stream_read_bytes;
                    luexp(stream->read(buf_u8, read_size, &stream_read_bytes));
                    real_read_bytes += stream_read_bytes;
                }
            }
            lucatchret;
            if(read_bytes) *read_bytes = real_read_bytes;
            return ok;
        }
        R<c32> StreamReadContext::read_one_char_from_stream()
        {
            c32 ret;
            lutry
            {
                if(encoding == Encoding::utf_8)
                {
                    c8 buf[4];
                    usize read_bytes;
                    luexp(stream_read(buf, sizeof(c8), &read_bytes));
                    if (read_bytes != sizeof(c8)) return 0;
                    const u8 first = (u8)buf[0];
                    usize charspan;
                    if(first <= 0x7F) charspan = 1;
                    else if(first >= 0xC2 && first <= 0xDF) charspan = 2;
                    else if(first >= 0xE0 && first <= 0xEF) charspan = 3;
                    else if(first >= 0xF0 && first <= 0xF4) charspan = 4;
                    else return BasicError::bad_data();
                    if (charspan > 1)
                    {
                        luexp(stream_read((buf + 1), sizeof(c8) * (charspan - 1), &read_bytes));
                        if (read_bytes != sizeof(c8) * (charspan - 1)) return BasicError::end_of_file();
                    }
                    R<c32> ch = utf8_decode_char(buf, charspan);
                    if(failed(ch)) return ch.errcode();
                    ret = ch.get();
                    if(!ret) return BasicError::bad_data();
                }
                else
                {
                    c16 buf[2];
                    usize read_bytes;
                    luexp(stream_read(buf, sizeof(c16), &read_bytes));
                    if (read_bytes != sizeof(c16)) return 0;
                    buf[0] = utf16_read_char(buf[0], encoding);
                    usize charspan = utf16_charlen(buf[0]);
                    if (charspan > 1)
                    {
                        luexp(stream_read((buf + 1), sizeof(c16) * (charspan - 1), &read_bytes));
                        if (read_bytes != sizeof(c16) * (charspan - 1)) return BasicError::end_of_file();
                        buf[1] = utf16_read_char(buf[1], encoding);
                    }
                    ret = utf16_decode_char(buf);
                    if(!ret) return BasicError::bad_data();
                }
            }
            lucatchret;
            return ret;
        }
        R<c32> StreamReadContext::next_char(usize index)
        {
            while (index >= buffer.size())
            {
                auto ch = read_one_char_from_stream();
                if (failed(ch)) return ch.errcode();
                if (!ch.get()) return (c32)0;
                buffer.push_back(ch.get());
            }
            return buffer[index];
        }
        RV StreamReadContext::skip_utf16_bom()
        {
            u8 ch[2];
            usize read_bytes;
            auto r = stream->read(ch, 2, &read_bytes);
            if(failed(r)) return r.errcode();
            if(read_bytes != 2)
            {
                if(read_bytes)
                {
                    stream_buffer.push_back(ch[0]);
                }
                return ok;
            }
            if(ch[0] == 0xFE && ch[1] == 0xFF)
            {
                encoding = Encoding::utf_16_be;
            }
            else if(ch[0] == 0xFF && ch[1] == 0xFE)
            {
                encoding = Encoding::utf_16_le;
            }
            else
            {
                stream_buffer.push_back(ch[0]);
                stream_buffer.push_back(ch[1]);
            }
            return ok;
        }
    }
}
