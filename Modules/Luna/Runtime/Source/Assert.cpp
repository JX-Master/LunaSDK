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
#include "../Log.hpp"
#include "../Debug.hpp"

namespace Luna
{
	LUNA_RUNTIME_API void assert_fail(const c8* msg, const c8* file, u32 line)
	{
		log(LogVerbosity::fatal_error, nullptr, "Assertion Failed: %s FILE: %s, LINE: %d", msg, file, line);
		void* stack_frames[256];
		u32 num_frames = stack_backtrace({stack_frames, 256});
		const c8** symbols = stack_backtrace_symbols({stack_frames, num_frames});
		log(LogVerbosity::fatal_error, nullptr, "Stack trace:");
		for(u32 i = 0; i < num_frames; ++i)
		{
			if(symbols[i]) log(LogVerbosity::fatal_error, nullptr, "%s", symbols[i]);
			else log(LogVerbosity::fatal_error, nullptr, "[Unnamed function]");
		}
		free_backtrace_symbols(symbols);
		OS::assert_fail(msg, file, line);
	}
	LUNA_RUNTIME_API void debug_break()
	{
		OS::debug_break();
	}
}
