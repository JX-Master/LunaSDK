/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Assert.cpp
* @author JXMaster
* @date 2020/12/20
 */
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "OS.hpp"
#include "../Assert.hpp"

namespace Luna
{
	LUNA_RUNTIME_API void assert_fail(const c8* msg, const c8* file, u32 line)
	{
		OS::assert_fail(msg, file, line);
	}
	LUNA_RUNTIME_API inline void debug_break()
	{
		OS::debug_break();
	}
}
