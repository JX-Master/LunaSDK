/*
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

#ifdef LUNA_RUNTIME_CHECK_MEMORY_LEAK
#include "../HashMap.hpp"
#include "../SpinLock.hpp"
#endif

namespace Luna
{
	usize g_allocated_memory = 0;

#ifdef LUNA_RUNTIME_CHECK_MEMORY_LEAK
	template <> struct hash<void*>
	{
		usize operator()(void* val) const { return reinterpret_cast<usize>(val); }
	};

	Unconstructed<HashMap<void*, const c8*, hash<void*>, equal_to<void*>, OSAllocator>> g_memory_leak_registry_map;
	Unconstructed<HashMap<void*, usize, hash<void*>, equal_to<void*>, OSAllocator>> g_memory_leap_map;
	SpinLock g_memory_leak_map_mtx;

	LUNA_RUNTIME_API void register_memory_block(void* blk, const c8* debug_name)
	{
		g_memory_leak_map_mtx.lock();
		g_memory_leak_registry_map.get().insert(make_pair(blk, debug_name));
		g_memory_leak_map_mtx.unlock();
	}
	LUNA_RUNTIME_API void unregister_memory_block(void* blk)
	{
		g_memory_leak_map_mtx.lock();
		g_memory_leak_registry_map.get().erase(blk);
		g_memory_leak_map_mtx.unlock();
	}
	void memory_check_init() 
	{
		g_memory_leak_registry_map.construct();
		g_memory_leap_map.construct();
	}
	void memory_check_close()
	{
		if (g_allocated_memory)
		{
			OS::debug_printf("[MEMORY LEAK CHECK]Memory leak detected: %llu bytes.\n", (u64)g_allocated_memory);
		}
		for (auto& i : g_memory_leap_map.get())
		{
			auto iter = g_memory_leak_registry_map.get().find(i.first);
			if (iter != g_memory_leak_registry_map.get().end())
			{
				OS::debug_printf("[MEMORY LEAK CHECK]Leaked block: 0x%016llx(%s), %llu bytes.\n", (u64)i.first, iter->second, i.second);
			}
			else
			{
				OS::debug_printf("[MEMORY LEAK CHECK]Leaked block: 0x%016llx(Unnamed), %llu bytes.\n", (u64)i.first, i.second);
			}
		}
		g_memory_leak_registry_map.destruct();
		g_memory_leap_map.destruct();
	}
#endif

	LUNA_RUNTIME_API void* memalloc(usize size, usize alignment)
	{
		void* mem = OS::memalloc(size, alignment);
		usize allocated = OS::memsize(mem, alignment);
		atom_add_usize(&g_allocated_memory, allocated);
#ifdef LUNA_RUNTIME_CHECK_MEMORY_LEAK
		g_memory_leak_map_mtx.lock();
		g_memory_leap_map.get().insert(make_pair(mem, allocated));
		g_memory_leak_map_mtx.unlock();
#endif
		return mem;
	}
	LUNA_RUNTIME_API void memfree(void* ptr, usize alignment)
	{
		atom_add_usize(&g_allocated_memory, -(isize)OS::memsize(ptr, alignment));
		OS::memfree(ptr, alignment);
#ifdef LUNA_RUNTIME_CHECK_MEMORY_LEAK
		g_memory_leak_map_mtx.lock();
		g_memory_leap_map.get().erase(ptr);
		g_memory_leak_map_mtx.unlock();
#endif
	}
	LUNA_RUNTIME_API void* memrealloc(void* ptr, usize size, usize alignment)
	{
		usize old_allocated = ptr ? OS::memsize(ptr, alignment) : 0;
		void* mem = OS::memrealloc(ptr, size, alignment);
		usize new_allocated = mem ? OS::memsize(mem, alignment) : 0;
		atom_add_usize(&g_allocated_memory, (isize)new_allocated - (isize)old_allocated);
#ifdef LUNA_RUNTIME_CHECK_MEMORY_LEAK
		g_memory_leak_map_mtx.lock();
		if (ptr)
		{
			g_memory_leap_map.get().erase(ptr);
		}
		if (mem)
		{
			g_memory_leap_map.get().insert(make_pair(mem, new_allocated));
		}
		g_memory_leak_map_mtx.unlock();
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