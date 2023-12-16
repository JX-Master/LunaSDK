/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Name.cpp
* @author JXMaster
* @date 2020/8/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Name.hpp"
#include "../Name.hpp"
#include "../SelfIndexedUnorderedMultiMap.hpp"
#include "../SpinLock.hpp"
#include "../Memory.hpp"
#include "../Profiler.hpp"
namespace Luna
{
	struct NameEntry
	{
		usize m_str_size;
		name_id_t m_id;
		u32 m_ref_count;

		NameEntry(name_id_t id, usize str_size, u32 ref_count) :
			m_id(id),
			m_str_size(str_size),
			m_ref_count(ref_count) {}
	};
	inline NameEntry* get_name_entry(const c8* name)
	{
		return ((NameEntry*)name) - 1;
	}
	inline const c8* get_name_string(NameEntry* entry)
	{
		return (const c8*)(entry + 1);
	}
	struct NameEntryExtractKey
	{
		const name_id_t operator()(NameEntry* v)
		{
			return v->m_id;
		}
	};
	Unconstructed<SelfIndexedUnorderedMultiMap<name_id_t, NameEntry*, NameEntryExtractKey>> g_name_map;
	RecursiveSpinLock g_name_mtx;
	bool g_name_inited = false;
	static void erase_entry(NameEntry* entry)
	{
		auto range = g_name_map.get().equal_range(entry->m_id);
		luassert(range.first != g_name_map.get().end());
		for (auto iter = range.first; iter != range.second; ++iter)
		{
			if (entry == *iter)
			{
				g_name_map.get().erase(iter);
				break;
			}
		}
		memfree(entry);
	}
	void name_init()
	{
		g_name_map.construct();
		g_name_inited = true;
	}
	void name_close()
	{
		// Release all name strings.
		for (auto& i : g_name_map.get())
		{
			memfree(i);
		}
		g_name_map.destruct();
		g_name_inited = false;
	}
	LUNA_RUNTIME_API const c8* intern_name(const c8* name)
	{
		lucheck_msg(g_name_inited, "intern_name must be called after Luna::init()!");
		if (!name || (*name == '\0')) return nullptr;
		return intern_name(name, strlen(name));
	}
	LUNA_RUNTIME_API const c8* intern_name(const c8* name, usize count)
	{
		lucheck_msg(g_name_inited, "intern_name must be called after Luna::init()!");
		if (!name || (*name == '\0')) return nullptr;
		name_id_t h = memhash<name_id_t>(name, count);
		LockGuard guard(g_name_mtx);
		auto range = g_name_map.get().equal_range(h);
		if (range.first != g_name_map.get().end())
		{
			auto iter = range.first;
			// Skip comparison for single-string case.
			++iter;
			if(iter == range.second)
			{
				iter = range.first;
				NameEntry* entry = *iter;
				atom_inc_u32(&(entry->m_ref_count));
				return get_name_string(entry);
			}
			// Compare each string to find the right one.
			for (iter = range.first; iter != range.second; ++iter)
			{
				NameEntry* entry = *iter;
				const c8* entry_string = get_name_string(entry);
				if (!memcmp(name, entry_string, count * sizeof(c8)))
				{
					atom_inc_u32(&(entry->m_ref_count));
					return entry_string;
				}
			}
		}
		// Create new entry.
		NameEntry* new_entry = (NameEntry*)memalloc(sizeof(c8) * (count + 1) + sizeof(NameEntry), alignof(NameEntry));
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		memory_profiler_set_memory_type(new_entry, "Name", 4);
#endif
		new (new_entry) NameEntry(h, count, 1);
		c8* buf = (c8*)(new_entry + 1);
		memcpy(buf, name, sizeof(c8) * count);
		buf[count] = 0;
		g_name_map.get().insert(new_entry);
		return buf;
	}
	LUNA_RUNTIME_API void retain_name(const c8* name)
	{
		if (!name) return;
		atom_inc_u32(&(get_name_entry(name)->m_ref_count));
	}
	LUNA_RUNTIME_API void release_name(const c8* name)
	{
		if(!g_name_inited) return;
		if (!name) return;
		NameEntry* entry = get_name_entry(name);
		u32 r = atom_dec_u32(&(entry->m_ref_count));
		if (!r)
		{
			LockGuard guard(g_name_mtx);
			erase_entry(entry);
		}
	}
	LUNA_RUNTIME_API name_id_t get_name_id(const c8* name)
	{
		if (!name) return 0;
		return get_name_entry(name)->m_id;
	}
	LUNA_RUNTIME_API usize get_name_size(const c8* name)
	{
		if (!name) return 0;
		return get_name_entry(name)->m_str_size;
	}
}