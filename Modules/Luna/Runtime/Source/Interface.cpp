/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Interface.cpp
* @author JXMaster
* @date 2022/5/27
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Interface.hpp"
#include "../Reflection.hpp"

namespace Luna
{
	LUNA_RUNTIME_API void impl_interface_for_type(const InterfaceImplDesc& desc)
	{
		auto type = get_type_by_guid(desc.type_guid);
		lucheck(type);
		InterfaceImplEntry* entry = (InterfaceImplEntry*)
			set_type_private_data(type, desc.interface_guid, sizeof(InterfaceImplEntry), alignof(InterfaceImplEntry));
		entry->cast_to_interface = desc.cast_to_interface;
	}
	LUNA_RUNTIME_API bool is_interface_implemented_by_type(typeinfo_t type, const Guid& iid)
	{
		InterfaceImplEntry* entry = (InterfaceImplEntry*)get_type_private_data(type, iid);
		return entry != nullptr;
	}
	LUNA_RUNTIME_API void* query_interface(object_t object, const Guid& iid)
	{
		typeinfo_t type = get_object_type(object);
		InterfaceImplEntry* entry = (InterfaceImplEntry*)get_type_private_data(type, iid);
		if (!entry) return nullptr;
		return entry->cast_to_interface(object);
	}
}