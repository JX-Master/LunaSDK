/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Utils.hpp
* @author JXMaster
* @date 2018/11/14
*/
#pragma once
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../../Unicode.hpp"
#include "../../OS.hpp"
namespace Luna
{
	namespace OS
	{
		inline wchar_t* utf8_to_wchar_buffered(const c8* src)
		{
			usize len = utf8_to_utf16_len(src) + 1;
			wchar_t* buf = (wchar_t*)memalloc(sizeof(wchar_t) * len);
			utf8_to_utf16((c16*)buf, len, src);
			return buf;
		}
	}
}