/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Memory.cpp
* @author JXMaster
* @date 2020/12/10
*/
#include "../PlatformDefines.hpp"

#define LUNA_RUNTIME_API LUNA_EXPORT

#include "../Memory.hpp"
#include "OS.hpp"
#include "../Atomic.hpp"
#include "Memory.hpp"
#include "../Profiler.hpp"

namespace Luna
{
	LUNA_RUNTIME_API void* memalloc(usize size, usize alignment)
	{
		void* mem = OS::memalloc(size, alignment);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		usize allocated = OS::memsize(mem, alignment);
		memory_profiler_allocate(mem, allocated);
#endif
		return mem;
	}
	LUNA_RUNTIME_API void memfree(void* ptr, usize alignment)
	{
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		usize allocated = OS::memsize(ptr, alignment);
		memory_profiler_deallocate(ptr, allocated);
#endif
		OS::memfree(ptr, alignment);
	}
	LUNA_RUNTIME_API void* memrealloc(void* ptr, usize size, usize alignment)
	{
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		usize allocated = OS::memsize(ptr, alignment);
#endif
		void* mem = OS::memrealloc(ptr, size, alignment);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		usize new_allocated = OS::memsize(mem, alignment);
		memory_profiler_reallocate(ptr, allocated, mem, new_allocated);
#endif
		return mem;
	}
	LUNA_RUNTIME_API usize memsize(void* ptr, usize alignment)
	{
		return OS::memsize(ptr, alignment);
	}
}