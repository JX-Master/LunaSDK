/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Serialization.hpp
* @author JXMaster
* @date 2022/5/23
* @brief The serialization API.
*/
#include "TypeInfo.hpp"
#include "Variant.hpp"
#include "Result.hpp"

namespace Luna
{
	using serialize_func_t = R<Variant>(typeinfo_t type, const void* inst);
	using deserialize_func_t = RV(typeinfo_t type, void* inst, const Variant& data);

	struct SerializableTypeDesc
	{
		serialize_func_t* serialize_func;
		deserialize_func_t* deserialize_func;
	};

	LUNA_RUNTIME_API bool is_type_serializable(typeinfo_t type);

	//! Sets one type to be serializable.
	//! @param[in] type The type to set.
	//! @param[in] desc The user-provided serialize and deserialize callback. If `nullptr` is specified,
	//! the system try to serialize the type bying serializing every property of the type.
	//! `nullptr` can only be specified if this is a structure type and all properties of this type is 
	//! serializable.
	LUNA_RUNTIME_API void set_serializable(typeinfo_t type, SerializableTypeDesc* desc = nullptr);

	template <typename _Ty>
	inline void set_serializable(SerializableTypeDesc* desc = nullptr)
	{
		set_serializable(typeof<_Ty>(), desc);
	}

	//! Serializes one value.
	LUNA_RUNTIME_API R<Variant> serialize(typeinfo_t type, const void* inst);

	template <typename _Ty>
	inline R<Variant> serialize(const _Ty& inst)
	{
		return serialize(typeof<_Ty>(), &inst);
	}

	//! Deserializes one value.
	LUNA_RUNTIME_API RV deserialize(typeinfo_t type, void* inst, const Variant& data);

	template <typename _Ty>
	inline RV deserialize(_Ty& inst, const Variant& data)
	{
		return deserialize(typeof<_Ty>(), &inst, data);
	}
}