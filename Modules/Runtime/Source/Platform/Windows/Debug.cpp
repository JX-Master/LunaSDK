/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.cpp
* @author JXMaster
* @date 2020/8/12
 */
#include <Runtime/PlatformDefines.hpp>

#ifdef LUNA_PLATFORM_WINDOWS

#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../OS.hpp"

namespace Luna
{
	namespace OS
	{
		void debug_printf(const c8* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			debug_vprintf(fmt, args);
			va_end(args);
		}
		void debug_vprintf(const c8* fmt, VarList args)
		{
			char buf[1024];
			vsnprintf_s(buf, 1024, fmt, args);
			::OutputDebugStringA(buf);
		}
	}
}

#endif