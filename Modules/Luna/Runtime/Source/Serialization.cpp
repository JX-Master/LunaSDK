/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Serialization.cpp
* @author JXMaster
* @date 2022/5/23
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Serialization.hpp"
#include "../Reflection.hpp"

namespace Luna
{
	constexpr Guid serialization_data_guid("{EAFCD4C8-1B75-434C-83AC-DE8C445BE688}");

	static R<Variant> default_structure_serialization(typeinfo_t type, const void* inst)
	{
		Variant ret(VariantType::object);
		lutry
		{
			auto properties = get_struct_properties(type);
			for(auto& prop : properties)
			{
				if (is_type_serializable(prop.type))
				{
					lulet(data, serialize(prop.type, (const void*)((usize)inst + prop.offset)));
					ret.insert(prop.name, move(data));
				}
			}
		}
		lucatchret;
		return ret;
	}
	static RV default_structure_deserialization(typeinfo_t type, void* inst, const Variant& data)
	{
		lutry
		{
			auto properties = get_struct_properties(type);
			for(auto& prop : properties)
			{
				auto& prop_data = data[prop.name];
				if (prop_data.valid() && is_type_serializable(prop.type))
				{
					luexp(deserialize(prop.type, (void*)((usize)inst + prop.offset), prop_data));
				}
			}
		}
		lucatchret;
		return ok;
	}
	static R<Variant> default_enum_serialization(typeinfo_t type, const void* inst)
	{
		i64 value = get_enum_instance_value(type, inst);
		auto options = get_enum_options(type);
		if (is_multienum_type(type))
		{
			Variant ret(VariantType::array);
			for(auto& desc : options)
			{
				if ((desc.value & value) != 0)
				{
					ret.push_back(desc.name);
				}
			}
			return ret;
		}
		else
		{
			for(auto& desc : options)
			{
				if (desc.value == value)
				{
					return Variant(desc.name);
				}
			}
			Name alias;
			Name name = get_type_name(type, &alias);
			if (alias)
			{
				return set_error(BasicError::bad_data(), "The value %lld is not a valid option for enumeration %s::%s.", value, name.c_str(), alias.c_str());
			}
			else
			{
				return set_error(BasicError::bad_data(), "The value %lld is not a valid option for enumeration %s.", value, name.c_str());
			}
		}
	}
	static RV default_enum_deserialization(typeinfo_t type, void* inst, const Variant& data)
	{
		auto options = get_enum_options(type);
		if (is_multienum_type(type))
		{
			i64 value = 0;
			for (auto& v : data.values())
			{
				for(auto& desc : options)
				{
					if (desc.name == v.str())
					{
						value |= desc.value;
						break;
					}
				}
			}
			set_enum_instance_value(type, inst, value);
		}
		else
		{
			bool found = false;
			for(auto& desc : options)
			{
				if (desc.name == data.str())
				{
					set_enum_instance_value(type, inst, desc.value);
					found = true;
					break;
				}
			}
			if (!found)
			{
				Name alias;
				Name name = get_type_name(type, &alias);
				if (alias)
				{
					return set_error(BasicError::bad_data(), "The value %s is not a valid option for enumeration %s::%s.", data.c_str(), name.c_str(), alias.c_str());
				}
				else
				{
					return set_error(BasicError::bad_data(), "The value %s is not a valid option for enumeration %s.", data.c_str(), name.c_str());
				}
			}
		}
		return ok;
	}
	LUNA_RUNTIME_API bool is_type_serializable(typeinfo_t type)
	{
		return check_type_attribute(type, "Serializable");
	}
	LUNA_RUNTIME_API void set_serializable(typeinfo_t type, SerializableTypeDesc* desc)
	{
		set_type_attribute(type, "Serializable");
		if (!desc)
		{
			if (is_struct_type(type))
			{
				SerializableTypeDesc* d = (SerializableTypeDesc*)set_type_private_data(type, serialization_data_guid, sizeof(SerializableTypeDesc));
				d->serialize_func = default_structure_serialization;
				d->deserialize_func = default_structure_deserialization;
			}
			else if (is_enum_type(type))
			{
				SerializableTypeDesc* d = (SerializableTypeDesc*)set_type_private_data(type, serialization_data_guid, sizeof(SerializableTypeDesc));
				d->serialize_func = default_enum_serialization;
				d->deserialize_func = default_enum_deserialization;
			}
		}
		else
		{
			SerializableTypeDesc* d = (SerializableTypeDesc*)set_type_private_data(type, serialization_data_guid, sizeof(SerializableTypeDesc));
			d->serialize_func = desc->serialize_func;
			d->deserialize_func = desc->deserialize_func;
		}
	}
	LUNA_RUNTIME_API R<Variant> serialize(typeinfo_t type, const void* inst)
	{
		SerializableTypeDesc* d = (SerializableTypeDesc*)get_type_private_data(type, serialization_data_guid);
		if (!d) return BasicError::not_supported();
		return d->serialize_func(type, inst);
	}
	LUNA_RUNTIME_API RV deserialize(typeinfo_t type, void* inst, const Variant& data)
	{
		SerializableTypeDesc* d = (SerializableTypeDesc*)get_type_private_data(type, serialization_data_guid);
		if (!d) return BasicError::not_supported();
		return d->deserialize_func(type, inst, data);
	}
}