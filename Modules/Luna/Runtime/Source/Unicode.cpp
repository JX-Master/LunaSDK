/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Unicode.hpp
* @author JXMaster
* @date 2020/2/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Unicode.hpp"

namespace Luna
{
    LUNA_RUNTIME_API R<usize> utf8_encode_char(c8* dst, usize dst_size, c32 ch)
    {
        usize num_bytes = utf8_charspan(ch);
        if(!num_bytes) return E_BAD_DATA;
        if(dst_size < num_bytes) return E_INSUFFICIENT_USER_BUFFER;
        lucheck(dst);
        if (ch <= UnicodeImpl::UTF8_ONE_END)
        {
            dst[0] = (c8)ch;
            return 1;
        }
        if (ch <= UnicodeImpl::UTF8_TWO_END)
        {
            dst[0] = (c8)(ch >> 6) + 0xC0;
            dst[1] = (c8)(ch & 0x3F) + 0x80;
            return 2;
        }
        if (ch <= UnicodeImpl::UTF8_THREE_END)
        {
            dst[0] = (c8)(ch >> 12) + 0xE0;
            dst[1] = ((c8)(ch >> 6) & 0x3F) + 0x80;
            dst[2] = (c8)(ch & 0x3F) + 0x80;
            return 3;
        }
        else
        {
            dst[0] = (c8)(ch >> 18) + 0xF0;
            dst[1] = ((c8)(ch >> 12) & 0x3F) + 0x80;
            dst[2] = ((c8)(ch >> 6) & 0x3F) + 0x80;
            dst[3] = (c8)(ch & 0x3F) + 0x80;
            return 4;
        }
    }
    LUNA_RUNTIME_API R<c32> utf8_decode_char(const c8* src, usize src_size, usize* out_num_bytes)
    {
        if(out_num_bytes) *out_num_bytes = 0;
        if(!src_size) return E_END_OF_FILE;
        lucheck(src);

        const u8 first = (u8)src[0];
        usize num_bytes;
        if(first <= 0x7F)
        {
            num_bytes = 1;
        }
        else if(first >= 0xC2 && first <= 0xDF) num_bytes = 2;
        else if(first >= 0xE0 && first <= 0xEF) num_bytes = 3;
        else if(first >= 0xF0 && first <= 0xF4) num_bytes = 4;
        else return E_BAD_DATA;

        for(usize i = 1; i < min(src_size, num_bytes); ++i)
        {
            const u8 continuation = (u8)src[i];
            if(continuation < 0x80 || continuation > 0xBF) return E_BAD_DATA;
        }

        if(src_size > 1 &&
            ((first == 0xE0 && (u8)src[1] < 0xA0) ||
            (first == 0xED && (u8)src[1] > 0x9F) ||
            (first == 0xF0 && (u8)src[1] < 0x90) ||
            (first == 0xF4 && (u8)src[1] > 0x8F)))
        {
            return E_BAD_DATA;
        }
        if(src_size < num_bytes) return E_END_OF_FILE;

        c32 codepoint;
        if(num_bytes == 1) codepoint = (c32)first;
        else if(num_bytes == 2)
        {
            codepoint = (c32)(((first & 0x1F) << 6) | ((u8)src[1] & 0x3F));
        }
        else if(num_bytes == 3)
        {
            codepoint = (c32)(((first & 0x0F) << 12) |
                (((u8)src[1] & 0x3F) << 6) |
                ((u8)src[2] & 0x3F));
        }
        else
        {
            codepoint = (c32)(((first & 0x07) << 18) |
                (((u8)src[1] & 0x3F) << 12) |
                (((u8)src[2] & 0x3F) << 6) |
                ((u8)src[3] & 0x3F));
        }
        if(out_num_bytes) *out_num_bytes = num_bytes;
        return codepoint;
    }

    LUNA_RUNTIME_API usize utf16_encode_char(c16* dst, c32 ch)
    {
        u32 code = (u32)ch;
        if (code >= 0xFFFF)
        {
            code -= 0x10000;
            dst[0] = (c16)(0xD800 + ((u16)(code >> 10) & 0x03FF));
            dst[1] = (c16)(0xDC00 + ((u16)code & 0x03FF));
            return 2;
        }
        else
        {
            dst[0] = (c16)code;
            return 1;
        }
    }

    LUNA_RUNTIME_API c32 utf16_decode_char(const c16* str)
    {
        u16 fc = (u16)*str;
        if (fc >= 0xD800 && fc <= 0xDBFF)
        {
            return (c32)(((u32)(fc - 0xD800) << 10) + (str[1] - 0xDC00) + 0x10000);
        }
        return (c32)fc;
    }

    LUNA_RUNTIME_API usize utf16_to_utf8(c8* dst, usize dst_max_chars, const c16* src, usize src_chars)
    {
        usize ri{ 0 };
        usize wi{ 0 };
        while(ri < src_chars && src[ri])
        {
            c32 ch = utf16_decode_char(src + ri);
            usize num_chars = utf8_charspan(ch);
            if(!num_chars || !dst_max_chars || wi + num_chars >= dst_max_chars)
            {
                break;
            }
            ri += utf16_charspan(ch);
            R<usize> encode_result = utf8_encode_char(dst + wi, dst_max_chars - wi, ch);
            if(failed(encode_result)) break;
            wi += encode_result.get();
        }
        if(dst_max_chars) dst[wi] = '\0';
        return wi;
    }

    LUNA_RUNTIME_API usize utf16_to_utf8_str(String& dst, const c16* src, usize src_chars)
    {
        usize len = utf16_to_utf8_len(src, src_chars);
        usize pos = dst.size();
        dst.resize(pos + len, 0);
        utf16_to_utf8(dst.data() + pos, len + 1, src, src_chars);
        return len;
    }

    LUNA_RUNTIME_API Array<c8> utf16_to_utf8_arr(const c16* src, usize src_chars)
    {
        usize len = utf16_to_utf8_len(src, src_chars);
        Array<c8> arr(len + 1);
        utf16_to_utf8(arr.data(), len + 1, src, src_chars);
        return arr;
    }

    LUNA_RUNTIME_API usize utf16_to_utf8_len(const c16* src, usize src_chars)
    {
        usize ri{ 0 };
        usize wi{ 0 };
        while(ri < src_chars && src[ri])
        {
            c32 ch = utf16_decode_char(src + ri);
            usize num_chars = utf8_charspan(ch);
            if(!num_chars) break;
            ri += utf16_charspan(ch);
            wi += num_chars;
        }
        return wi;
    }

    LUNA_RUNTIME_API usize utf8_to_utf16(c16* dst, usize dst_max_chars, const c8* src, usize src_chars)
    {
        usize src_size = 0;
        while(src_size < src_chars && src[src_size]) ++src_size;
        usize ri{ 0 };
        usize wi{ 0 };
        while(ri < src_size)
        {
            usize num_bytes;
            R<c32> ch = utf8_decode_char(src + ri, src_size - ri, &num_bytes);
            if(failed(ch)) break;
            usize num_chars = utf16_charspan(ch.get());
            if (wi + num_chars >= dst_max_chars)
            {
                break;
            }
            ri += num_bytes;
            wi += utf16_encode_char(dst + wi, ch.get());
        }
        if(dst_max_chars) dst[wi] = '\0';
        return wi;
    }

    LUNA_RUNTIME_API usize utf8_to_utf16_str(String16& dst, const c8* src, usize src_chars)
    {
        usize len = utf8_to_utf16_len(src, src_chars);
        usize pos = dst.size();
        dst.resize(pos + len, 0);
        utf8_to_utf16(dst.data() + pos, len + 1, src, src_chars);
        return len;
    }

    LUNA_RUNTIME_API Array<c16> utf8_to_utf16_arr(const c8* src, usize src_chars)
    {
        usize len = utf8_to_utf16_len(src, src_chars);
        Array<c16> arr(len + 1);
        utf8_to_utf16(arr.data(), len + 1, src, src_chars);
        return arr;
    }

    LUNA_RUNTIME_API usize utf8_to_utf16_len(const c8* src, usize src_max_chars)
    {
        usize src_size = 0;
        while(src_size < src_max_chars && src[src_size]) ++src_size;
        usize ri{ 0 };
        usize wi{ 0 };
        while(ri < src_size)
        {
            usize num_bytes;
            R<c32> ch = utf8_decode_char(src + ri, src_size - ri, &num_bytes);
            if(failed(ch)) break;
            ri += num_bytes;
            wi += utf16_charspan(ch.get());
        }
        return wi;
    }
}
