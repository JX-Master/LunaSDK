/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Unicode.hpp
* @author JXMaster
* @date 2018/10/26
 */
#pragma once
#include "Base.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	namespace UnicodeImpl
	{
		/*
		UTF8 Encoding:

		U+00000000~U+0000007F	7	0	0XXXXXXX
		U+00000080~U+000007FF	11	C0	110XXXXX 10XXXXXX
		U+00000800~U+0000FFFF	16	E0	1110XXXX 10XXXXXX 10XXXXXX
		U+00010000~U+001FFFFF	21	F0	11110XXX 10XXXXXX 10XXXXXX 10XXXXXX
		U+00200000~U+03FFFFFF	26	F8	111110XX 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX
		U+04000000~U+7FFFFFFF	31	FC	1111110X 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX
		*/

		// Unicode range.
		constexpr c32 UTF8_ONE_START = 0x0000;
		constexpr c32 UTF8_ONE_END = 0x007F;
		constexpr c32 UTF8_TWO_START = 0x0080;
		constexpr c32 UTF8_TWO_END = 0x07FF;
		constexpr c32 UTF8_THREE_START = 0x0800;
		constexpr c32 UTF8_THREE_END = 0xFFFF;
		constexpr c32 UTF8_FOUR_START = 0x00010000;
		constexpr c32 UTF8_FOUR_END = 0x001FFFFF;
		constexpr c32 UTF8_FIVE_START = 0x00200000;
		constexpr c32 UTF8_FIVE_END = 0x03FFFFFF;
		constexpr c32 UTF8_SIX_START = 0x04000000;
		constexpr c32 UTF8_SIX_END = 0x7FFFFFFF;
	}

	//! Returns the number of `char` characters needed to store the Unicode char `ch` in UTF-8 encoding.
	inline constexpr usize utf8_charspan(c32 ch)
	{
		if (ch <= UnicodeImpl::UTF8_ONE_END)
		{
			return 1;
		}
		if (ch <= UnicodeImpl::UTF8_TWO_END)
		{
			return 2;
		}
		if (ch <= UnicodeImpl::UTF8_THREE_END)
		{
			return 3;
		}
		if (ch <= UnicodeImpl::UTF8_FOUR_END)
		{
			return 4;
		}
		if (ch <= UnicodeImpl::UTF8_FIVE_END)
		{
			return 5;
		}
		// The maximum number is 0x7FFFFFFF.
		return 6;
	}

	//! Returns the number of `c8` characters the next Unicode char takes from the first `c8` character.
	inline constexpr usize utf8_charlen(c8 ch)
	{
		if ((u8)ch <= 127)	// 0XXXXXXXb
		{
			return 1;
		}
		if ((u8)ch <= 223)	// 110XXXXXb
		{
			return 2;
		}
		if ((u8)ch <= 239)	// 1110XXXXb
		{
			return 3;
		}
		if ((u8)ch <= 247)	// 11110XXXb
		{
			return 4;
		}
		if ((u8)ch <= 251)	// 111110XXb
		{
			return 5;
		}
		// 1111110Xb
		// The maximum number that may occur is 253(11111101b)
		// FE and FF never appear.
		return 6;
	}

	//! Returns the number of `c8` characters the next Unicode char takes in the specified UTF-8 string.
	inline constexpr usize utf8_charlen(const c8* src)
	{
		return utf8_charlen(src[0]);
	}

	//! Checks the number of Unicode characters in a null-terminated utf-8 string, 
	//! not including the null terminator.
	inline constexpr usize utf8_strlen(const c8* src)
	{
		usize l{ 0 };
		while (*src)
		{
			src += utf8_charlen(src);
			++l;
		}
		return l;
	}

	//! Returns the position of the first `char` character for the `n`th Unicode character.
	inline constexpr usize utf8_index(const c8* str, usize n)
	{
		usize i{ 0 };
		usize p{ 0 };
		while (str[p] && i < n)
		{
			p += utf8_charlen(str + p);
			++i;
		}
		return p;
	}

	//! Encodes the Unicode character `ch` into 1~6 `char` bytes in the `dst` UTF-8 string.
	//! Returns the number of `char` characters printed to the `dst` buffer.
	LUNA_RUNTIME_API usize utf8_encode_char(c8* dst, c32 ch);

	//! Returns the Unicode character in the UTF-8 string `str`.
	LUNA_RUNTIME_API c32 utf8_decode_char(const c8* str);

	//! Returns the number of `c16` characters needed to store the Unicode char `ch` in UTF-16 encoding.
	inline constexpr usize utf16_charspan(c32 ch)
	{
		if (ch <= 0xFFFF)
		{
			return 1;
		}
		return 2;
	}

	//! Returns the number of `c16` characters the next Unicode char takes from the first `c16` character.
	inline constexpr usize utf16_charlen(c16 ch)
	{
		if (ch >= 0xD800 && ch <= 0xDFFF)
		{
			return 2;
		}
		return 1;
	}
	
	//! Returns the number of `c16` characters the next Unicode char takes in the specified UTF-16 string.
	inline constexpr usize utf16_charlen(const c16* src)
	{
		return utf16_charlen(src[0]);
	}

	//! Checks the number of Unicode characters in a null-terminated utf-16 string, 
	//! not including the null terminator.
	inline constexpr usize utf16_strlen(const c16* src)
	{
		usize l{ 0 };
		while (*src)
		{
			src += utf16_charlen(src);
			++l;
		}
		return l;
	}

	//! Returns the position of the first `c16` character for the `n`th Unicode character.
	inline constexpr usize utf16_index(const c16* str, usize n)
	{
		usize i{ 0 };
		usize p{ 0 };
		while (str[p] && i < n)
		{
			p += utf16_charlen(str + p);
			++i;
		}
		return p;
	}

	//! Encodes the Unicode character `ch` into 1~2 `c16` bytes in the `dst` UTF-16 string.
	//! Returns the number of `c16` characters printed to the `dst` buffer.
	LUNA_RUNTIME_API usize utf16_encode_char(c16* dst, c32 ch);

	//! Returns the Unicode character in the UTF-16 string `str`.
	LUNA_RUNTIME_API c32 utf16_decode_char(const c16* str);

	//! Convert a utf-16 string to utf-8 string in destination buffer.
	//! @param[in] dst The buffer to hold the output string.
	//! @param[in] dst_max_chars The maximum characters the `dst` buffer can hold, 
	//! including the null-terminator.
	//! @param[in] src The null-terminated buffer holding the source string.
	//! @param[in] src_chars The maximum characters to read. Specify `USIZE_MAX` to read till the end of the 
	//! string.
	//! @return Returns the number of characters outputted to the `dst` buffer, 
	//! not including the null-terminator.
	LUNA_RUNTIME_API usize utf16_to_utf8(c8* dst, usize dst_max_chars, const c16* src, usize src_chars = USIZE_MAX);

	//! Determines the length of the corresponding utf-8 string for a utf-16 input string,
	//! not include the null-terminator.
	LUNA_RUNTIME_API usize utf16_to_utf8_len(const c16* src, usize src_chars = USIZE_MAX);

	//! Convert a utf-8 string to utf-16 string in destination buffer.
	//! @param[in] dst The buffer to hold the output string.
	//! @param[in] dst_max_chars The maximum characters the `dst` buffer can hold, 
	//! including the null-terminator.
	//! @param[in] src The null-terminated buffer holding the source string.
	//! @param[in] src_chars The maximum characters to read. Specify `USIZE_MAX` to read till the end of the 
	//! string.
	//! @return Returns the number of characters outputted to the `dst` buffer, 
	//! not including the null-terminator. Returns (usize)-1 if the input string is not
	//! in utf-8 format.
	LUNA_RUNTIME_API usize utf8_to_utf16(c16* dst, usize dst_max_chars, const c8* src, usize src_chars = USIZE_MAX);

	//! Determines the length of the corresponding utf-16 string for a utf-8 input string,
	//! not include the null-terminator.
	//! @return Returns length of the `src` string in utf-16 format, not including the null-terminator. 
	//! Returns (usize)-1 if the `src` string is not in utf-8 format.
	LUNA_RUNTIME_API usize utf8_to_utf16_len(const c8* src, usize src_chars = USIZE_MAX);
}
