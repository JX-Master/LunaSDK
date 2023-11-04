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
	usize g_allocated_memory = 0;

	LUNA_RUNTIME_API void* memalloc(usize size, usize alignment)
	{
		void* mem = OS::memalloc(size, alignment);
		usize allocated = OS::memsize(mem, alignment);
		atom_add_usize(&g_allocated_memory, allocated);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		memory_profiler_allocate(mem, size);
#endif
		return mem;
	}
	LUNA_RUNTIME_API void memfree(void* ptr, usize alignment)
	{
		atom_add_usize(&g_allocated_memory, -(isize)OS::memsize(ptr, alignment));
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		memory_profiler_deallocate(ptr);
#endif
		OS::memfree(ptr, alignment);
	}
	LUNA_RUNTIME_API void* memrealloc(void* ptr, usize size, usize alignment)
	{
		usize old_allocated = ptr ? OS::memsize(ptr, alignment) : 0;
		void* mem = OS::memrealloc(ptr, size, alignment);
		usize new_allocated = mem ? OS::memsize(mem, alignment) : 0;
		atom_add_usize(&g_allocated_memory, (isize)new_allocated - (isize)old_allocated);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		memory_profiler_reallocate(ptr, mem, size);
#endif
		return mem;
	}
	LUNA_RUNTIME_API usize memsize(void* ptr, usize alignment)
	{
		return OS::memsize(ptr, alignment);
	}
	LUNA_RUNTIME_API usize get_allocated_memory()
	{
		return g_allocated_memory;
	}
}