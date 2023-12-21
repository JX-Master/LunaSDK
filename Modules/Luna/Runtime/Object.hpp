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
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeObject Boxed objects
	//! @}

	//! @addtogroup RuntimeObject
	//! @{

	//! The opaque pointer that points to the boxed object.
	using object_t = opaque_t;

	//! The reference counter type for boxed objects.
	using ref_count_t = i32;

	//! Registers one type so that it can be used for creating boxed objects.
	//! @details This function only registers basic information for one type, it does not register properties, constructors and other information.
	//! Use @ref register_struct_type if you want a type with full reflection info.
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

	//! Allocates one boxed object.
	//! @param[in] type The type of the object to allocate.
	//! @return Returns one pointer to the allocated object.
	//! The returned object is not initialized, the user should call constructors of the type manually.
	//! The returned object has 1 strong reference and 0 weak reference.
	LUNA_RUNTIME_API object_t object_alloc(typeinfo_t type);

	//! Increases the strong refernece counter value by one.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the strong reference counter value of the object after the operation.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API ref_count_t object_retain(object_t object_ptr);

	//! Decreases the strong refernece counter value by one, and destroys the object if the reference counter drops to 0.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the strong reference counter value of the object after the operation.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API ref_count_t object_release(object_t object_ptr);

	//! Fetches the strong refernece counter value of the boxed object.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the strong reference counter value of the object.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API ref_count_t object_ref_count(object_t object_ptr);

	//! Increases the weak refernece counter value by one.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the weak reference counter value of the object after the operation.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API ref_count_t object_retain_weak(object_t object_ptr);

	//! Decreases the weak refernece counter value by one.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the weak reference counter value of the object after the operation.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API ref_count_t object_release_weak(object_t object_ptr);

	//! Fetches the weak refernece counter value of the boxed object.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the weak reference counter value of the object.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API ref_count_t object_weak_ref_count(object_t object_ptr);

	//! Checks if the boxed object is expired, that is, destructed but its memeory is not freed.
	//! @details One object will be expired if its strong reference counter value drops 0, but its weak reference counter value is not 0.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns `true` if the object is expired, returns `false` otherwise.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API bool object_expired(object_t object_ptr);

	//! Increases the strong refernece counter value by one if the boxed object is not expired.
	//! @details This call is atomic and can be used to create strong references from weak references.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns `true` if the object is valid and the strong reference counter value is successfully increased, 
	//! returns `false` otherwise.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API bool object_retain_if_not_expired(object_t object_ptr);

	//! Gets the type object of the boxed object.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns the type object of the boxed object.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API typeinfo_t get_object_type(object_t object_ptr);

	//! Checks whether the boxed object is the specified type or derived types of the specified type.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns `true` if the boxed object is the specified type or derived types of the specified type, returns `false` otherwise.
	//! @par Valid Usage
	//! * `object_ptr` must points to one memory returned by @ref object_alloc.
	LUNA_RUNTIME_API bool object_is_type(object_t object_ptr, typeinfo_t type);

	//! Casts the object to the specified type.
	//! @param[in] object_ptr The object pointer.
	//! @return Returns `object_ptr` casted to the specified type if type casting is succeeded, returns `nullptr` otherwise.
	template <typename _Rty>
	inline _Rty* cast_object(object_t object_ptr)
	{
		return object_is_type(object_ptr, get_type_by_guid(_Rty::__guid)) ? (_Rty*)object_ptr : nullptr;
	}

	//! @}
}
