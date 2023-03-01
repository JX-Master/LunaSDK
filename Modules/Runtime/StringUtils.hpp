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
    using std::strncpy;
	using std::strcat;
	using std::strncat;
	using std::strxfrm;
	using std::strncmp;
	using std::strcoll;
	using std::strchr;
	using std::strrchr;
	using std::strspn;
	using std::strcspn;
	using std::strpbrk;
	using std::strstr;
	using std::strtok;

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

    template <typename _CharT>
	inline usize strlen(const _CharT* s)
	{
		const _CharT* end = s;
		while (*end++ != 0);
		return end - s - 1;
	}

	template <typename _CharT>
	inline _CharT* strcpy(_CharT* dest, const _CharT* src)
	{
		_CharT* t = dest;
		while (*src)
		{
			*t = *src;
			++t;
			++src;
		}
		*t = (_CharT)0;
		return dest;
	}

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
    
    inline i64 strtoi64(const c8* str, c8** str_end, i32 base)
    {
        return std::strtoll(str, str_end, base);
    }
    inline u64 strtou64(const c8* str, c8** str_end, i32 base)
    {
        return std::strtoull(str, str_end, base);
    }
    inline f64 strtof64(const c8* str, c8** str_end)
    {
        return std::strtod(str, str_end);
    }
}