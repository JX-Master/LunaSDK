/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Name.cpp
* @author JXMaster
* @date 2020/8/8
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Name.hpp"
#include "../Name.hpp"
#include "../SelfIndexedUnorderedMultiMap.hpp"
#include "../SpinLock.hpp"
#include "../Memory.hpp"
namespace Luna
{
	struct NameEntry
	{
		NameEntry* m_cache_list_prior;
		NameEntry* m_cache_list_next;
		c8* m_str;
		usize m_str_size;
		name_id_t m_id;
		u32 m_ref_count;

		NameEntry(name_id_t id, c8* str, usize str_size, u32 ref_count) :
			m_id(id),
			m_cache_list_prior(nullptr),
			m_cache_list_next(nullptr),
			m_str(str),
			m_str_size(str_size),
			m_ref_count(ref_count) {}
	};
	struct NameEntryExtractKey
	{
		const name_id_t operator()(const NameEntry& v)
		{
			return v.m_id;
		}
	};
	Unconstructed<SelfIndexedUnorderedMultiMap<name_id_t, NameEntry, NameEntryExtractKey>> g_name_map;
	NameEntry* g_name_cache_list_head; // Caches names whose reference count is 0. These names will cache a short period of time to prevent being interned/released frequently.
	NameEntry* g_name_cache_list_tail;
	usize g_name_retain_list_size;
	SpinLock g_name_mtx;
	static void add_name_to_cache_list(NameEntry* entry)
	{
		// Add to head.
		if (g_name_cache_list_head)
		{
			g_name_cache_list_head->m_cache_list_prior = entry;
		}
		entry->m_cache_list_next = g_name_cache_list_head;
		g_name_cache_list_head = entry;
		if (!g_name_cache_list_tail) // If the list is empty...
		{
			g_name_cache_list_tail = entry;
		}
		++g_name_retain_list_size;
	}
	static void remove_name_from_cache_list(NameEntry* entry)
	{
		if (entry->m_cache_list_prior)
		{
			entry->m_cache_list_prior->m_cache_list_next = entry->m_cache_list_next;
		}
		if (entry->m_cache_list_next)
		{
			entry->m_cache_list_next->m_cache_list_prior = entry->m_cache_list_prior;
		}
		if (g_name_cache_list_head == entry)
		{
			g_name_cache_list_head = entry->m_cache_list_next;
		}
		if (g_name_cache_list_tail == entry)
		{
			g_name_cache_list_tail = entry->m_cache_list_prior;
		}
		entry->m_cache_list_next = nullptr;
		entry->m_cache_list_prior = nullptr;
		--g_name_retain_list_size;
	}
	static void erase_entry(NameEntry* entry)
	{
		memfree(((NameEntry**)(entry->m_str)) - 1);
		auto range = g_name_map.get().equal_range(entry->m_id);
		luassert(range.first != g_name_map.get().end());
		for (auto iter = range.first; iter != range.second; ++iter)
		{
			if (entry == &(*iter))
			{
				g_name_map.get().erase(iter);
				return;
			}
		}
		luassert(false);
	}
	static void name_cache_list_cleanup()
	{
		// TODO: Tweak these two values based on the real situation.
		constexpr usize cleanup_threshold = 1000;				 // The maximum number of entries to cache.
		constexpr usize max_cleanup_count_per_call = 50;		 // The maximum number of entries to free at each cache call.
		if (g_name_retain_list_size <= cleanup_threshold) return;
		usize cleanup_count = min(g_name_retain_list_size - cleanup_threshold, max_cleanup_count_per_call);
		// Cleanup is performed from tail to head since unused names will bubble to tail as time goes by.
		NameEntry* entry = g_name_cache_list_tail;
		for(usize i = 0; i < cleanup_count; ++i)
		{
			NameEntry* next = entry->m_cache_list_prior;
			erase_entry(entry);
			entry = next;
		}
		entry->m_cache_list_next = nullptr;
		g_name_cache_list_tail = entry;
		g_name_retain_list_size -= cleanup_count;
	}
	void name_init()
	{
		g_name_map.construct();
		g_name_cache_list_head = nullptr;
		g_name_cache_list_tail = nullptr;
		g_name_retain_list_size = 0;
	}
	void name_close()
	{
		// Release all name strings.
		for (auto& i : g_name_map.get())
		{
			memfree(((NameEntry**)(i.m_str)) - 1);
		}
		g_name_map.destruct();
	}
	LUNA_RUNTIME_API const c8* intern_name(const c8* name)
	{
		if (!name || (*name == '\0')) return nullptr;
		return intern_name(name, strlen(name));
	}
	LUNA_RUNTIME_API const c8* intern_name(const c8* name, usize count)
	{
		if (!name || (*name == '\0')) return nullptr;
		name_id_t h = memhash<name_id_t>(name, count);
		LockGuard guard(g_name_mtx);
		auto range = g_name_map.get().equal_range(h);
		if (range.first != g_name_map.get().end())
		{
			// Compare each string to find the right one.
			for (auto iter = range.first; iter != range.second; ++iter)
			{
				if (!memcmp(name, iter->m_str, count * sizeof(c8)))
				{
					if (!(iter->m_ref_count))
					{
						remove_name_from_cache_list(&(*iter));
					}
					++(iter->m_ref_count);
					return iter->m_str;
				}
			}
		}
		// Create new entry.
		name_cache_list_cleanup();
		NameEntry** name_buf = (NameEntry**)memalloc(sizeof(c8) * (count + 1) + sizeof(NameEntry**));
		c8* buf = (c8*)(name_buf + 1);
		memcpy(buf, name, sizeof(c8) * count);
		buf[count] = 0;
		auto iter = g_name_map.get().insert(NameEntry(h, buf, count, 1));
		*name_buf = &(*iter);
		return buf;
	}
	inline NameEntry* get_name_entry(const c8* name)
	{
		if (!name)
		{
			return nullptr;
		}
		return *(((NameEntry**)name) - 1);
	}
	LUNA_RUNTIME_API void retain_name(const c8* name)
	{
		if (!name) return;
		LockGuard guard(g_name_mtx);
		NameEntry* entry = get_name_entry(name);
		if (!entry) return;
		if (!entry->m_ref_count)
		{
			remove_name_from_cache_list(entry);
		}
		++entry->m_ref_count;
	}
	LUNA_RUNTIME_API void release_name(const c8* name)
	{
		if (!name) return;
		LockGuard guard(g_name_mtx);
		NameEntry* entry = get_name_entry(name);
		if (!entry) return;
		--entry->m_ref_count;
		if (!entry->m_ref_count)
		{
			add_name_to_cache_list(entry);
		}
	}
	LUNA_RUNTIME_API name_id_t get_name_id(const c8* name)
	{
		if (!name) return 0;
		LockGuard guard(g_name_mtx);
		NameEntry* entry = get_name_entry(name);
		return entry->m_id;
	}
	LUNA_RUNTIME_API usize get_name_size(const c8* name)
	{
		return get_name_entry(name)->m_str_size;
	}
}