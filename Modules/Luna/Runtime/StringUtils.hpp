/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StringUtils.hpp
* @author JXMaster
* @date 2023/2/28
* @brief String processing utility library.
 */
#pragma once
#include "Base.hpp"

namespace Luna
{
	//! @addtogroup RuntimeString
	//! @{

	//! Computes the length (number of characters) of the specified string by searching for
	//! the first null character in the string.
	//! @param[in] str The string to be examined.
	//! @return Returns the length of the string. The length does not include the null terminator.
	//! @par Valid Usage
	//! * `str` must specify one null-terminated string.
	template <typename _CharT>
	inline usize strlen(const _CharT* str)
	{
		const _CharT* end = str;
		while (*end++ != 0);
		return end - str - 1;
	}

	//! Computes the length (number of characters) of the specified string by searching for
	//! the first null character in the string. The search process stops after reading `max_chars` 
	//! characters.
	//! @param[in] str The string to be examined.
	//! @param[in] max_chars The maximum number of characters to examine, including the null terminator.
	//! @return Returns the length of the string. The length does not include the null terminator.
	//! Returns `0` if `s` is `nullptr`.
	//! Returns `max_chars` if the null character is not found in the first `max_chars` characters
	//! of `s`.
	template <typename _CharT>
	inline usize strnlen(const _CharT* str, usize max_chars)
	{
		if(!str) return 0;
		const _CharT* end = str;
		while (*end && max_chars)
		{
			++end;
			--max_chars;
		}
		return end - str;
	}
	
	//! Copies at most `max_chars` characters from `src` to `dst`, including the null terminator.
	//! @details The real number of characters copied from `src` will be `min(max_chars - 1, strlen(dst))`.
	//! The null terminator will then be written at the end of the copied characters.
	//! @param[in] dst The character buffer to copy to.
	//! @param[in] src The character buffer to copy from.
	//! @param[in] max_chars The maximum number of characters that can be written to `dst`, including the 
	//! null terminator.
	//! @return Returns `dst`.
	template <typename _CharT>
	_CharT* strncpy(_CharT* dst, const _CharT* src, usize max_chars)
	{
		if(!max_chars) return dst;
		--max_chars; // for null terminator.
		_CharT* t = dst;
		while (*src && max_chars)
		{
			*t = *src;
			++t;
			++src;
			--max_chars;
		}
		*t = (_CharT)0;
		return dst;
	}

	//! Compares characters in two strings.
	//! @param[in] lhs The first string to compare.
	//! @param[in] rhs The second string to compare.
	//! @return Returns `0` if both character sequences compare equivalent.
	//! 
	//! Returns negative value if `lhs` appears before the character sequence specified by `rhs`, in lexicographical order.
	//! 
	//! Returns positive value if `lhs` appears after the character sequence specified by `rhs`, in lexicographical order.
	//! @par Valid Usage
	//! * `lhs` and `rhs` must specify null-terminated strings.
	template <typename _CharT>
	inline i32 strcmp(const _CharT* lhs, const _CharT* rhs)
	{
		_CharT l = *lhs;
		_CharT r = *rhs;
		while (l && (l == r))
		{
			++lhs;
			++rhs;
			l = *lhs;
			r = *rhs;
		}
		return (i32)l - (i32)r;
	}

	//! Compares at most `max_chars` characters in two strings.
	//! @param[in] lhs The first string to compare.
	//! @param[in] rhs The second string to compare.
	//! @param[in] max_chars The maximum number of characters to compare.
	//! @return Returns `0` if both character sequences compare equivalent, or if `max_chars` is `0`.
	//! 
	//! Returns negative value if `lhs` appears before the character sequence specified by `rhs`, in lexicographical order.
	//! 
	//! Returns positive value if `lhs` appears after the character sequence specified by `rhs`, in lexicographical order.
	template <typename _CharT>
	inline i32 strncmp(const _CharT* lhs, const _CharT* rhs, usize max_chars)
	{
		if(!max_chars) return 0;
		_CharT l = *lhs;
		_CharT r = *rhs;
		--max_chars;
		while (l && (l == r) && max_chars)
		{
			++lhs;
			++rhs;
			l = *lhs;
			r = *rhs;
			--max_chars;
		}
		return (i32)l - (i32)r;
	}

	//! Finds the first occurrence of `ch` in the null-terminated byte string pointed to by `str`.
	//! @details The terminating null character is considered to be a part of the string and can be found if searching for `\0`.
	//! @param[in] str The string to be examined.
	//! @param[in] ch The character to search for.
	//! @return Returns one pointer to the found character in `str`. Returns `nullptr` if not found.
	//! @par Valid Usage
	//! * `str` must specify one null-terminated string.
	template <typename _CharT>
	inline const _CharT* strchr(const _CharT* str, _CharT ch)
	{
		_CharT cmp = *str;
		if(cmp == ch) return str;
		while(cmp)
		{
			++str;
			cmp = *str;
			if(cmp == ch) return str;
		}
		return nullptr;
	}

	//! Finds the first occurrence of `ch` in the null-terminated byte string pointed to by `str`.
	//! @details The terminating null character is considered to be a part of the string and can be found if searching for `\0`.
	//! @param[in] str The string to be examined.
	//! @param[in] ch The character to search for.
	//! @return Returns one pointer to the found character in `str`. Returns `nullptr` if not found.
	//! @par Valid Usage
	//! * `str` must specify one null-terminated string.
	template <typename _CharT>
	inline _CharT* strchr(_CharT* str, _CharT ch)
	{
		_CharT cmp = *str;
		if(cmp == ch) return str;
		while(cmp)
		{
			++str;
			cmp = *str;
			if(cmp == ch) return str;
		}
		return nullptr;
	}

	//! Finds the last occurrence of `ch` in the null-terminated byte string pointed to by `str`.
	//! @details The terminating null character is considered to be a part of the string and can be found if searching for `\0`.
	//! @param[in] str The string to be examined.
	//! @param[in] ch The character to search for.
	//! @return Returns one pointer to the found character in `str`. Returns `nullptr` if not found.
	//! @par Valid Usage
	//! * `str` must specify one null-terminated string.
	template <typename _CharT>
	inline const _CharT* strrchr(const _CharT* str, _CharT ch)
	{
		_CharT cmp = *str;
		_CharT ret = nullptr;
		if(cmp == ch) ret = str;
		while(cmp)
		{
			++str;
			cmp = *str;
			if(cmp == ch) ret = str;
		}
		return ret;
	}

	//! Finds the last occurrence of `ch` in the null-terminated byte string pointed to by `str`.
	//! @details The terminating null character is considered to be a part of the string and can be found if searching for `\0`.
	//! @param[in] str The string to be examined.
	//! @param[in] ch The character to search for.
	//! @return Returns one pointer to the found character in `str`. Returns `nullptr` if not found.
	//! @par Valid Usage
	//! * `str` must specify one null-terminated string.
	template <typename _CharT>
	inline _CharT* strrchr(_CharT* str, _CharT ch)
	{
		_CharT cmp = *str;
		_CharT ret = nullptr;
		if(cmp == ch) ret = str;
		while(cmp)
		{
			++str;
			cmp = *str;
			if(cmp == ch) ret = str;
		}
		return ret;
	}

	//! Finds the first occurrence of the specified substring in the null-terminated byte string pointed to by `str`.
	//! @param[in] str The string to be examined.
	//! @param[in] substr The substring to search for.
	//! @return Returns one pointer to the first character of the found substring in `str`. Returns `nullptr` if not found.
	//! @par Valid Usage
	//! * Both `str` and `substr` must specify null-terminated strings.
	template <typename _CharT>
	inline const _CharT* strstr(const _CharT* str, const _CharT* substr)
	{
		usize len = strlen(str);
		usize plen = strlen(substr);
		auto iter = search(str, str + len, substr, substr + plen);
		return iter == str + len ? nullptr : iter;
	}

	//! Finds the first occurrence of the specified substring in the null-terminated byte string pointed to by `str`.
	//! @param[in] str The string to be examined.
	//! @param[in] substr The substring to search for.
	//! @return Returns one pointer to the first character of the found substring in `str`. Returns `nullptr` if not found.
	//! @par Valid Usage
	//! * Both `str` and `substr` must specify null-terminated strings.
	template <typename _CharT>
	inline _CharT* strstr(_CharT* str, const _CharT* substr)
	{
		usize len = strlen(str);
		usize plen = strlen(substr);
		auto iter = search(str, str + len, substr, substr + plen);
		return iter == str + len ? nullptr : iter;
	}
    
	//! Interprets an integer value in a string pointed to by `str`.
	//! @param[in] str The pointer to the null-terminated byte string to be interpreted.
	//! @param[out] str_end If not `nullptr`, returns the pointer to the end of the integer value string being interpreted.
	//! @param[in] base The encoding base for the interpreted integer value.
	//! @return Returns the converted value.
	//! If the converted value falls out of range of corresponding return type, @ref I64_MIN or @ref I64_MAX is returned.
	//! If no conversion can be performed, `0`​ is returned.
    inline i64 strtoi64(const c8* str, c8** str_end, i32 base)
    {
        return std::strtoll(str, str_end, base);
    }

	//! Interprets an unsigned integer value in a string pointed to by `str`.
	//! @param[in] str The pointer to the null-terminated byte string to be interpreted.
	//! @param[out] str_end If not `nullptr`, returns the pointer to the end of the unsigned integer value string being interpreted.
	//! @param[in] base The encoding base for the interpreted unsiged integer value.
	//! @return Returns the converted value.
	//! If the converted value falls out of range of corresponding return type, `0` or @ref U64_MAX is returned.
	//! If no conversion can be performed, `0`​ is returned.
    inline u64 strtou64(const c8* str, c8** str_end, i32 base)
    {
        return std::strtoull(str, str_end, base);
    }

	//! Interprets a floating-point value in a string pointed to by `str`.
	//! @param[in] str The pointer to the null-terminated byte string to be interpreted.
	//! @param[out] str_end If not `nullptr`, returns the pointer to the end of the floating-point value string being interpreted.
	//! @return Returns the converted value.
	//! If the converted value falls out of range of corresponding return type, @ref F64_INFINITY is returned.
	//! If no conversion can be performed, `0`​ is returned.
    inline f64 strtof64(const c8* str, c8** str_end)
    {
        return std::strtod(str, str_end);
    }

	//! Interprets a floating-point value in a string pointed to by `str`.
	//! @param[in] str The pointer to the null-terminated byte string to be interpreted.
	//! @param[out] str_end If not `nullptr`, returns the pointer to the end of the floating-point value string being interpreted.
	//! @return Returns the converted value.
	//! If the converted value falls out of range of corresponding return type, @ref F32_INFINITY is returned.
	//! If no conversion can be performed, `0`​ is returned.
    inline f32 strtof32(const c8* str, c8** str_end)
    {
        return std::strtof(str, str_end);
    }

	using std::isalnum;
    using std::isalpha;
    using std::islower;
    using std::isupper;
    using std::isdigit;
    using std::isxdigit;
    using std::iscntrl;
    using std::isgraph;
    using std::isspace;
    using std::isblank;
    using std::isprint;
    using std::ispunct;
    using std::tolower;
    using std::toupper;

	//! @}
}