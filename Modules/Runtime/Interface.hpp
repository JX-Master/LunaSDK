/*
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
	struct InterfaceImplDesc
	{
		Guid type_guid;
		Guid interface_guid;
		void* (*cast_to_interface)(object_t obj);
	};

	LUNA_RUNTIME_API void impl_interface_for_type(const InterfaceImplDesc& desc);

	LUNA_RUNTIME_API bool is_interface_implemented_by_type(typeinfo_t type, const Guid& iid);

	template <typename _Ty>
	void impl_interface_for_type() {}

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

	LUNA_RUNTIME_API void* query_interface(object_t object, const Guid& iid);

	template <typename _Ity>
	_Ity* query_interface(object_t object)
	{
		return object ? (_Ity*)query_interface(object, _Ity::__guid) : nullptr;
	}

	struct Interface
	{
		//! Gets the object that implements the interface.
		virtual object_t get_object() = 0;
	};
}
