/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Assert.hpp
* @author JXMaster
* @date 2018/10/26
*/
#include "../../OS.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <assert.h>

namespace Luna
{
    namespace OS
    {
		void assert_fail(const c8* msg, const c8* file, u32 line)
		{
#ifdef LUNA_DEBUG
			usize msg_len = utf8_to_utf16_len(msg);
			usize file_len = utf8_to_utf16_len(file);
			c16* wbuf = (c16*)alloca(sizeof(c16) * (msg_len + 1));
			c16* wfile = (c16*)alloca(sizeof(c16) * (file_len + 1));
			utf8_to_utf16(wbuf, msg_len + 1, msg);
			utf8_to_utf16(wfile, file_len + 1, file);
			_wassert((wchar_t*)wbuf, (wchar_t*)wfile, line);
#else
			printf("Assertion Failed: %s FILE: %s, LINE: %d", msg, file, line);
			abort();
#endif
		}

		void debug_break()
		{
#ifdef LUNA_DEBUG
			__debugbreak();
#endif
		}
    }

}