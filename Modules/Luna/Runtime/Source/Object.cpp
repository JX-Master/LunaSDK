/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Object.cpp
* @author JXMaster
* @date 2022/3/10
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Object.hpp"
#include "../Atomic.hpp"
#include "../HashMap.hpp"
#include "../UniquePtr.hpp"
#include "../Vector.hpp"
#include "../SpinLock.hpp"
#include "../Profiler.hpp"

#include "OS.hpp"

namespace Luna
{
	struct ObjectHeader
	{
		typeinfo_t type;
		ref_count_t ref_count;
		ref_count_t weak_ref_count;
		u32 expired;
		ObjectHeader() :
			ref_count(1)
			, weak_ref_count(0)
			, expired(0)
		{}
		~ObjectHeader() {}
		object_t get_object() const
		{
			return (void*)((usize)this + sizeof(ObjectHeader));
		}
		//! We need to allocate extra `n` alignment sizes to store the object header.
		static usize get_padding_size(usize alignment)
		{
			// We reserve n * alignment paddings until we have enough room to hold ObjectHeader.
			usize padding_count = sizeof(ObjectHeader) / alignment + ((sizeof(ObjectHeader) % alignment) ? 1 : 0);
			return padding_count * alignment;
		}
		void expire()
		{
			if (!atom_exchange_u32(&expired, 1))
			{
				// We use number 2 to identify the object is destroying, so present the calls to `destroy`
				// to free this object. This may happen when object A holds a strong
				// reference to object B and object B holds a weak reference to A, when A is expiring, it
				// releases the reference to B, so B is destroyed and releases the reference to A, and makes
				// weak and strong reference count of A being 0, which will trigger the `destroy` in 
				// `release_weak`, and then triggers `destroy` again after A is destructed in `release`.
				atom_exchange_u32(&expired, 2);
				destruct_type(type, get_object());
				atom_exchange_u32(&expired, 1);
			}
		}
		void destroy()
		{
			if (expired != 2)
			{
				this->~ObjectHeader();
				object_t obj = get_object();
				usize alignment = get_type_alignment(type);
				usize padded_size = get_padding_size(alignment);
				void* raw_ptr = (void*)((usize)obj - padded_size);
				memfree(raw_ptr, alignment);
			}
		}
	};
	inline ObjectHeader* get_header(object_t object)
	{
		return (ObjectHeader*)(((usize)object) - sizeof(ObjectHeader));
	}
	LUNA_RUNTIME_API object_t object_alloc(typeinfo_t type)
	{
		usize size = get_type_size(type);
		usize alignment = get_type_alignment(type);
		usize padding_size = ObjectHeader::get_padding_size(alignment);
		void* mem = memalloc(size + padding_size, alignment);
		object_t object = (object_t)((usize)mem + padding_size);
		ObjectHeader* header = get_header(object);
		new (header) ObjectHeader();
		header->type = type;
#ifdef LUNA_MEMORY_PROFILER_ENABLED
		Name type_name = get_type_name(type);
		memory_profiler_set_memory_type(mem, type_name.c_str(), type_name.size());
#endif
		return object;
	}

	LUNA_RUNTIME_API ref_count_t object_retain(object_t object_ptr)
	{
		return atom_inc_i32(&(get_header(object_ptr)->ref_count));
	}
	LUNA_RUNTIME_API ref_count_t object_release(object_t object_ptr)
	{
		ObjectHeader* header = get_header(object_ptr);
		ref_count_t r = atom_dec_i32(&(header->ref_count));
		if (!r)
		{
			header->expire();
			if (!header->weak_ref_count)
			{
				header->destroy();
			}
		}
		return r;
	}
	LUNA_RUNTIME_API ref_count_t object_ref_count(object_t object_ptr)
	{
		return get_header(object_ptr)->ref_count;
	}
	LUNA_RUNTIME_API ref_count_t object_retain_weak(object_t object_ptr)
	{
		return atom_inc_i32(&(get_header(object_ptr)->weak_ref_count));
	}
	LUNA_RUNTIME_API ref_count_t object_release_weak(object_t object_ptr)
	{
		ObjectHeader* header = get_header(object_ptr);
		ref_count_t r = atom_dec_i32(&(header->weak_ref_count));
		if (!r && !(header->ref_count))
		{
			header->destroy();
		}
		return r;
	}
	LUNA_RUNTIME_API ref_count_t object_weak_ref_count(object_t object_ptr)
	{
		return get_header(object_ptr)->weak_ref_count;
	}
	LUNA_RUNTIME_API bool object_expired(object_t object_ptr)
	{
		return get_header(object_ptr)->expired ? true : false;
	}
	LUNA_RUNTIME_API bool object_retain_if_not_expired(object_t object_ptr)
	{
		ObjectHeader* header = get_header(object_ptr);
		while (true)
		{
			ref_count_t tmp = static_cast<ref_count_t const volatile&>(header->ref_count);
			if (!tmp) return false;
			if (atom_compare_exchange_i32(&(header->ref_count), tmp + 1, tmp) == tmp) return true;
		}
	}
	LUNA_RUNTIME_API typeinfo_t get_object_type(object_t object_ptr)
	{
		return get_header(object_ptr)->type;
	}
	LUNA_RUNTIME_API bool object_is_type(object_t object_ptr, typeinfo_t type)
	{
		if (!type) return false;
		typeinfo_t obj_type = get_object_type(object_ptr);
		while (obj_type)
		{
			if (type == obj_type) return true;
			obj_type = get_base_type(obj_type);
		}
		return false;
	}
	void object_close()
	{
	}
}