/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Object.hpp
* @author JXMaster
* @date 2022/3/10
*/
#pragma once
#include "Base.hpp"
#include "Name.hpp"
#include "Atomic.hpp"
#include "Reflection.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! The opaque pointer that points to the managed object memory.
	using object_t = opaque_t;

	using ref_count_t = i32;

	template <typename _Ty>
	typeinfo_t register_boxed_type()
	{
		StructureTypeDesc desc;
		desc.guid = _Ty::__guid;
		desc.name = _Ty::__name;
		desc.size = sizeof(_Ty);
		desc.alignment = alignof(_Ty);
		desc.ctor = nullptr;
		desc.dtor = default_dtor<_Ty>;
		desc.copy_ctor = nullptr;
		desc.move_ctor = nullptr;
		desc.copy_assign = nullptr;
		desc.move_assign = nullptr;
		return register_struct_type(desc);
	}

	LUNA_RUNTIME_API object_t object_alloc(typeinfo_t type);

	LUNA_RUNTIME_API ref_count_t object_retain(object_t object_ptr);

	LUNA_RUNTIME_API ref_count_t object_release(object_t object_ptr);

	LUNA_RUNTIME_API ref_count_t object_ref_count(object_t object_ptr);

	LUNA_RUNTIME_API ref_count_t object_retain_weak(object_t object_ptr);

	LUNA_RUNTIME_API ref_count_t object_release_weak(object_t object_ptr);

	LUNA_RUNTIME_API ref_count_t object_weak_ref_count(object_t object_ptr);

	LUNA_RUNTIME_API bool object_expired(object_t object_ptr);

	LUNA_RUNTIME_API bool object_retain_if_not_expired(object_t object_ptr);

	LUNA_RUNTIME_API typeinfo_t get_object_type(object_t object_ptr);

	LUNA_RUNTIME_API bool object_is_type(object_t object_ptr, typeinfo_t type);
}
