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
		if(!size) return nullptr;
		void* mem = OS::memalloc(size, alignment);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		usize allocated = OS::memsize(mem, alignment);
		memory_profiler_allocate(mem, allocated);
#endif
		return mem;
	}
	LUNA_RUNTIME_API void* memrealloc(void* ptr, usize size, usize alignment)
	{
		if(!ptr) return memalloc(size, alignment);
		// freeing memroy if `size` is `0`.
		if(!size)
		{
			memfree(ptr, alignment);
			return nullptr;
		}
		// expanding or contracting the existing area pointed to by `ptr`, if possible.
		usize old_size = memsize(ptr, alignment);
		if(size <= old_size) return ptr;
		// reallocating.
		void* new_ptr = memalloc(size, alignment);
		memcpy(new_ptr, ptr, min(old_size, size));
		memfree(ptr, alignment);
		return new_ptr;
	}
	LUNA_RUNTIME_API void memfree(void* ptr, usize alignment)
	{
		if(!ptr) return;
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		memory_profiler_deallocate(ptr);
#endif
		OS::memfree(ptr, alignment);
	}
	LUNA_RUNTIME_API usize memsize(void* ptr, usize alignment)
	{
		return OS::memsize(ptr, alignment);
	}
}