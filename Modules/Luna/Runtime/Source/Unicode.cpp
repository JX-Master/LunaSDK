/*!
* This file is a portion of Luna SDK.
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
	LUNA_RUNTIME_API usize utf8_encode_char(c8* dst, c32 ch)
	{
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
		if (ch <= UnicodeImpl::UTF8_FOUR_END)
		{
			dst[0] = (c8)(ch >> 18) + 0xF0;
			dst[1] = ((c8)(ch >> 12) & 0x3F) + 0x80;
			dst[2] = ((c8)(ch >> 6) & 0x3F) + 0x80;
			dst[3] = (c8)(ch & 0x3F) + 0x80;
			return 4;
		}
		if (ch <= UnicodeImpl::UTF8_FIVE_END)
		{
			dst[0] = (c8)(ch >> 24) + 0xF8;
			dst[1] = ((c8)(ch >> 18) & 0x3F) + 0x80;
			dst[2] = ((c8)(ch >> 12) & 0x3F) + 0x80;
			dst[3] = ((c8)(ch >> 6) & 0x3F) + 0x80;
			dst[4] = (c8)(ch & 0x3F) + 0x80;
			return 5;
		}
		// if (ch <= l_utf8_six_end)
		{
			dst[0] = (c8)(ch >> 30) + 0xFC;
			dst[1] = ((c8)(ch >> 24) & 0x3F) + 0x80;
			dst[2] = ((c8)(ch >> 18) & 0x3F) + 0x80;
			dst[3] = ((c8)(ch >> 12) & 0x3F) + 0x80;
			dst[4] = ((c8)(ch >> 6) & 0x3F) + 0x80;
			dst[5] = (c8)(ch & 0x3F) + 0x80;
			return 6;
		}
	}
	LUNA_RUNTIME_API c32 utf8_decode_char(const c8* str)
	{
		{
			u8 fc = (u8)str[0];
			if (fc <= 127)
			{
				return (c32)str[0];
			}
			if (fc <= 223)
			{
				return ((str[0] & 0x1F) << 6) + (str[1] & 0x3F);
			}
			if (fc <= 239)
			{
				return ((str[0] & 0x0F) << 12) + ((str[1] & 0x3F) << 6) + (str[2] & 0x3F);
			}
			if (fc <= 247)
			{
				return ((str[0] & 0x07) << 18) + ((str[1] & 0x3F) << 12) + ((str[2] & 0x3F) << 6) + (str[3] & 0x3F);
			}
			if (fc <= 251)
			{
				return ((str[0] & 0x03) << 24) + ((str[1] & 0x3F) << 18) + ((str[2] & 0x3F) << 12) + ((str[3] & 0x3F) << 6) + (str[4] & 0x3F);
			}
			return ((str[0] & 0x01) << 30) + ((str[1] & 0x3F) << 24) + ((str[2] & 0x3F) << 18) + ((str[3] & 0x3F) << 12) + ((str[4] & 0x3F) << 6) + (str[5] & 0x3F);
		}
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
		while ((src[ri]) && (ri < src_chars))
		{
			c32 ch = utf16_decode_char(src + ri);
			usize num_chars = utf8_charspan(ch);
			if (wi + num_chars > dst_max_chars - 1)
			{
				break;
			}
			ri += utf16_charspan(ch);
			wi += utf8_encode_char(dst + wi, ch);
		}
		dst[wi] = '\0';
		return wi;
	}

	LUNA_RUNTIME_API usize utf16_to_utf8_len(const c16* src, usize src_chars)
	{
		usize ri{ 0 };
		usize wi{ 0 };
		while (src[ri] && (ri < src_chars))
		{
			c32 ch = utf16_decode_char(src + ri);
			ri += utf16_charspan(ch);
			wi += utf8_charspan(ch);
		}
		return wi;
	}

	LUNA_RUNTIME_API usize utf8_to_utf16(c16* dst, usize dst_max_chars, const c8* src, usize src_chars)
	{
		usize ri{ 0 };
		usize wi{ 0 };
		while ((src[ri]) && (ri < src_chars))
		{
			c32 ch = utf8_decode_char(src + ri);
			usize num_chars = utf16_charspan(ch);
			if (wi + num_chars >= dst_max_chars)
			{
				break;
			}
			ri += utf8_charspan(ch);
			wi += utf16_encode_char(dst + wi, ch);
		}
		dst[wi] = '\0';
		return wi;
	}

	LUNA_RUNTIME_API usize utf8_to_utf16_len(const c8* src, usize src_max_chars)
	{
		usize ri{ 0 };
		usize wi{ 0 };
		while (src[ri] && (ri < src_max_chars))
		{
			c32 ch = utf8_decode_char(src + ri);
			ri += utf8_charspan(ch);
			wi += utf16_charspan(ch);
		}
		return wi;
	}
}