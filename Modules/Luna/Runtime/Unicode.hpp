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

	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeUnicode Unicode encoding/decoding
	//! @}

	//! @addtogroup RuntimeUnicode
	//! @{

	//! Gets the number of UTF-8 characters needed to store the Unicode char in UTF-8 encoding.
	//! @param[in] ch The Unicode codepoint of the character.
	//! @return Returns the number of UTF-8 characters needed to store the Unicode char in UTF-8 encoding.
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

	//! Gets the number of UTF-8 characters the Unicode character takes from the first UTF-8 character.
	//! @param[in] ch The first UTF-8 character of the Unicode character.
	//! @return Returns the number of UTF-8 characters the Unicode character takes.
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

	//! Gets the number of UTF-8 characters the Unicode character takes from the first UTF-8 character.
	//! @param[in] src The pointer that points to the first UTF-8 character of the Unicode character.
	//! @return Returns the number of UTF-8 characters the Unicode character takes.
	inline constexpr usize utf8_charlen(const c8* src)
	{
		return utf8_charlen(src[0]);
	}

	//! Gets the number of Unicode characters in a null-terminated UTF-8 string, not including the null terminator.
	//! @param[in] src The UTF-8 string to check.
	//! @return Returns the number of Unicode characters in the UTF-8 string.
	//! @par Valid Usage
	//! * `src` must points to a valid, null-terminated UTF-8 string.
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

	//! Gets the index of the first UTF-8 character for the specified Unicode character.
	//! @param[in] str The UTF-8 string to check.
	//! @param[in] n The index of the Unicode character to check.
	//! @return Returns the index of the first UTF-8 character for the specified Unicode character.
	//! @par Valid Usage
	//! * `src` must points to a valid UTF-8 string.
	//! * `n` must be smaller than the number of Unicode characters in the string.
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

	//! Encodes the Unicode character into 1~6 UTF-8 characters.
	//! @param[in] dst The buffer for writing encoded UTF-8 characters.
	//! @param[in] ch The Unicode character to encode.
	//! @return Returns the number of UTF-8 characters written to `dst`.
	//! @par Valid Usage
	//! * `dst` must be large enough to hold all UTF-8 characters written. 
	//! The user can use @ref utf8_charspan to check the required space in advance.
	LUNA_RUNTIME_API usize utf8_encode_char(c8* dst, c32 ch);

	//! Decodes one Unicode character from 1~6 UTF-8 characters.
	//! @param[in] str The pointer that points to UTF-8 characters to decode.
	//! @return Returns the decoded Unicode character.
	//! @par Valid Usage
	//! * `str` must points to a valid UTF-8 character string.
	LUNA_RUNTIME_API c32 utf8_decode_char(const c8* str);

	//! Gets the number of UTF-16 characters needed to store the Unicode char in UTF-16 encoding.
	//! @param[in] ch The Unicode codepoint of the character.
	//! @return Returns the number of UTF-16 characters needed to store the Unicode char in UTF-16 encoding.
	inline constexpr usize utf16_charspan(c32 ch)
	{
		if (ch <= 0xFFFF)
		{
			return 1;
		}
		return 2;
	}

	//! Gets the number of UTF-16 characters the Unicode character takes from the first UTF-16 character.
	//! @param[in] ch The first UTF-16 character of the Unicode character.
	//! @return Returns the number of UTF-16 characters the Unicode character takes.
	inline constexpr usize utf16_charlen(c16 ch)
	{
		if (ch >= 0xD800 && ch <= 0xDFFF)
		{
			return 2;
		}
		return 1;
	}
	
	//! Gets the number of UTF-16 characters the Unicode character takes from the first UTF-16 character.
	//! @param[in] src The pointer that points to the first UTF-16 character of the Unicode character.
	//! @return Returns the number of UTF-16 characters the Unicode character takes.
	inline constexpr usize utf16_charlen(const c16* src)
	{
		return utf16_charlen(src[0]);
	}

	//! Gets the number of Unicode characters in a null-terminated UTF-16 string, not including the null terminator.
	//! @param[in] src The UTF-16 string to check.
	//! @return Returns the number of Unicode characters in the UTF-16 string.
	//! @par Valid Usage
	//! * `src` must points to a valid, null-terminated UTF-16 string.
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

	//! Gets the index of the first UTF-16 character for the specified Unicode character.
	//! @param[in] str The UTF-16 string to check.
	//! @param[in] n The index of the Unicode character to check.
	//! @return Returns the index of the first UTF-16 character for the specified Unicode character.
	//! @par Valid Usage
	//! * `src` must points to a valid UTF-16 string.
	//! * `n` must be smaller than the number of Unicode characters in the string.
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

	//! Encodes the Unicode character into 1~2 UTF-16 characters using platform-native endian.
	//! @param[in] dst The buffer for writing encoded UTF-16 characters.
	//! @param[in] ch The Unicode character to encode.
	//! @return Returns the number of UTF-16 characters written to `dst`.
	//! @par Valid Usage
	//! * `dst` must be large enough to hold all UTF-16 characters written. 
	//! The user can use @ref utf16_charspan to check the required space in advance.
	LUNA_RUNTIME_API usize utf16_encode_char(c16* dst, c32 ch);

	//! Decodes one Unicode character from 1~2 UTF-16 characters.
	//! @param[in] str The pointer that points to UTF-16 characters to decode.
	//! @return Returns the decoded Unicode character.
	//! @par Valid Usage
	//! * `str` must points to a valid UTF-16 character string in platform-native endian.
	LUNA_RUNTIME_API c32 utf16_decode_char(const c16* str);

	//! Converts a UTF-16 string to UTF-8 string.
	//! @param[in] dst The buffer to hold the output string.
	//! @param[in] dst_max_chars The maximum characters the `dst` buffer can hold, including the null-terminator.
	//! @param[in] src The buffer holding the source string.
	//! @param[in] src_chars The maximum characters to read. Specify `USIZE_MAX` to read till the end of the 
	//! string. The conversion process will stop on first null terminator, or when `src_chars` is reached.
	//! @return Returns the number of characters outputted to the `dst` buffer, not including the null-terminator.
	//! @par Valid Usage
	//! * `src` must be a valid UTF-16 string.
	LUNA_RUNTIME_API usize utf16_to_utf8(c8* dst, usize dst_max_chars, const c16* src, usize src_chars = USIZE_MAX);

	//! Determines the length of the corresponding UTF-8 string for a UTF-16 string, not include the null-terminator.
	//! @param[in] src The UTF-16 string to check.
	//! @param[in] src_chars The maximum characters to read. Specify `USIZE_MAX` to read till the end of the 
	//! string. The checking process will stop on first null terminator, or when `src_chars` is reached.
	//! @return Returns the length of the corresponding UTF-8 string for a UTF-16 string, not include the null-terminator.
	LUNA_RUNTIME_API usize utf16_to_utf8_len(const c16* src, usize src_chars = USIZE_MAX);

	//! Converts a UTF-8 string to UTF-16 string.
	//! @param[in] dst The buffer to hold the output string.
	//! @param[in] dst_max_chars The maximum characters the `dst` buffer can hold, including the null-terminator.
	//! @param[in] src The holding the source string.
	//! @param[in] src_chars The maximum characters to read. Specify `USIZE_MAX` to read till the end of the 
	//! string. The conversion process will stop on first null terminator, or when `src_chars` is reached.
	//! @return Returns the number of characters outputted to the `dst` buffer, not including the null-terminator.
	//! @par Valid Usage
	//! * `src` must be a valid UTF-8 string.
	LUNA_RUNTIME_API usize utf8_to_utf16(c16* dst, usize dst_max_chars, const c8* src, usize src_chars = USIZE_MAX);

	//! Determines the length of the corresponding UTF-16 string for a UTF-8 string, not include the null-terminator.
	//! @param[in] src The UTF-8 string to check.
	//! @param[in] src_chars The maximum characters to read. Specify `USIZE_MAX` to read till the end of the 
	//! string. The checking process will stop on first null terminator, or when `src_chars` is reached.
	//! @return Returns the length of the corresponding UTF-16 string for a UTF-8 string, not include the null-terminator.
	LUNA_RUNTIME_API usize utf8_to_utf16_len(const c8* src, usize src_chars = USIZE_MAX);

	//! @}
}
