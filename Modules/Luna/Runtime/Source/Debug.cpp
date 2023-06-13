/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Debug.cpp
* @author JXMaster
* @date 2020/4/1
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Debug.hpp"
#include "OS.hpp"

namespace Luna
{
	LUNA_RUNTIME_API void debug_printf(const c8* fmt, ...)
	{
		VarList args;
		va_start(args, fmt);
		debug_vprintf(fmt, args);
		va_end(args);
	}

	LUNA_RUNTIME_API void debug_vprintf(const c8* fmt, VarList args)
	{
		OS::debug_vprintf(fmt, args);
	}
}