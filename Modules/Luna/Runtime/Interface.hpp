/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Interface.hpp
* @author JXMaster
* @date 2022/5/27
*/
#pragma once
#include "TypeInfo.hpp"
#include "Name.hpp"
#include "Object.hpp"

#define luiid(_guid) static constexpr Luna::Guid __guid { Luna::Guid(u8##_guid) };
#define luiimpl() virtual object_t get_object() override { return this; }

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeInterface Interfaces
	//! @}

	//! @addtogroup RuntimeInterface
	//! @{
	
	//! Describes arguments to register an interface implementation.
	struct InterfaceImplDesc
	{
		//! The GUID of the type that implements the interface.
		Guid type_guid;
		//! The GUID of the interface.
		Guid interface_guid;
		//! The function pointer called to cast the object pointer to the specified interface pointer.
		void* (*cast_to_interface)(object_t obj);
	};

	//! Registers one interface implementation.
	//! @param[in] desc The interface implementation descriptor.
	LUNA_RUNTIME_API void impl_interface_for_type(const InterfaceImplDesc& desc);

	template <typename _Ty>
	void impl_interface_for_type() {}

	//! Registers one or more interface implementations for one type.
	//! @remark To register one or multiple interfaces for one type in one call, use the template version of `impl_interface_for_type` like so:
	//! ```
	//! impl_interface_for_type<Type, Interface1, Interface2, Interface3>();
	//! ```
	//! @ref Interface should not be included when using `impl_interface_for_type`. The template version of `impl_interface_for_type` 
	//! calls the non-template version automatically, so it will be more convenient than calling the non-template version directly.
	template <typename _Ty, typename _Ity1, typename... _Itys>
	void impl_interface_for_type()
	{
		InterfaceImplDesc desc;
		desc.type_guid = _Ty::__guid;
		desc.interface_guid = _Ity1::__guid;
		desc.cast_to_interface = [](object_t obj) {return (void*)(static_cast<_Ity1*>((_Ty*)obj)); };
		impl_interface_for_type(desc);
		impl_interface_for_type<_Ty, _Itys...>();
	}

	//! Checks whether the specified type implements the specified interface.
	//! @param[in] type The type to query.
	//! @param[in] iid The interface GUID to query.
	//! @return Returns `true` if the specified type implements the specified interface. Returns `false` otherwise.
	LUNA_RUNTIME_API bool is_interface_implemented_by_type(typeinfo_t type, const Guid& iid);

	//! Gets one interface pointer from one pointer to one boxed object that implements the interface.
	//! @param[in] object The pointer to the boxed object to query interface from.
	//! @param[in] iid The interface GUID to query.
	//! @return Returns one pointer that can be safely reinterpreted to the specified interface pointer.
	//! Returns `nullptr` if the specified interface is not implemented by the specified boxed object.
	LUNA_RUNTIME_API void* query_interface(object_t object, const Guid& iid);

	//! Gets one interface pointer from one pointer to one boxed object that implements the interface.
	//! @param[in] object The pointer to the boxed object to query interface from.
	//! @return Returns the specified interface pointer for the boxed object.
	//! Returns `nullptr` if the specified interface is not implemented by the specified boxed object.
	//! @remark The template version of `query_interface` provides a more convenient way to fetch one interface pointer
	//! from one boxed object pointer like so:
	//! ```
	//! object_t object = ...;
	//! Interface1* i = query_interface<Interface1>(object);
	//! ```
	template <typename _Ity>
	_Ity* query_interface(object_t object)
	{
		return object ? (_Ity*)query_interface(object, _Ity::__guid) : nullptr;
	}

	//! @interface Interface
	//! The base interface for all other interfaces in Luna SDK.
	struct Interface
	{
		//! Gets the pointer to the boxed object that implements this interface.
		//! @return Returns the pointer to the boxed object.
		virtual object_t get_object() = 0;
	};

	//! @}
}
