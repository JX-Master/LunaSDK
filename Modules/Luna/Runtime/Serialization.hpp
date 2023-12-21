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
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeSerialization Serialization
	//! @}

	//! @addtogroup RuntimeSerialization
	//! @{
	
	//! The serialization function for one instance.
	//! @param[in] type The type of the instance.
	//! @param[in] inst The instance data.
	//! @return Returns one variant that stores the serialized data.
	using serialize_func_t = R<Variant>(typeinfo_t type, const void* inst);

	//! The deserialization function for one instance.
	//! @param[in] type The type of the instance.
	//! @param[in] inst The instance data.
	//! @param[in] data The serialized data used for deserialization.
	using deserialize_func_t = RV(typeinfo_t type, void* inst, const Variant& data);

	//! Describes one serializable type.
	struct SerializableTypeDesc
	{
		//! The serialization function of the type.
		serialize_func_t* serialize_func;
		//! The deserialization function of the type.
		deserialize_func_t* deserialize_func;
	};

	//! Checks whether one type is serializable.
	//! @param[in] type The type to check.
	//! @return Returns `ture` if the type is serializable. Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_serializable(typeinfo_t type);

	//! Sets one type to be serializable.
	//! @param[in] type The type to set.
	//! @param[in] desc The user-provided serialize and deserialize callback. If `nullptr` is specified,
	//! the system try to serialize the type bying serializing every property of the type.
	//! `nullptr` can only be specified if this is a structure type and all properties of this type is 
	//! serializable.
	LUNA_RUNTIME_API void set_serializable(typeinfo_t type, SerializableTypeDesc* desc = nullptr);

	//! Sets one type `_Ty` to be serializable.
	//! @param[in] desc The user-provided serialize and deserialize callback. If `nullptr` is specified,
	//! the system try to serialize the type bying serializing every property of the type.
	//! `nullptr` can only be specified if this is a structure type and all properties of this type is 
	//! serializable.
	template <typename _Ty>
	inline void set_serializable(SerializableTypeDesc* desc = nullptr)
	{
		set_serializable(typeof<_Ty>(), desc);
	}

	//! Serializes one instance.
	//! @param[in] type The type of the instance.
	//! @param[in] inst The instance data.
	//! @return Returns one variant that stores the serialized data.
	LUNA_RUNTIME_API R<Variant> serialize(typeinfo_t type, const void* inst);

	//! Serializes one instance of type `_Ty`.
	//! @param[in] inst The instance data.
	//! @return Returns one variant that stores the serialized data.
	template <typename _Ty>
	inline R<Variant> serialize(const _Ty& inst)
	{
		return serialize(typeof<_Ty>(), &inst);
	}

	//! Deserializes one value.
	//! @param[in] type The type of the instance.
	//! @param[in] inst The instance data.
	//! @param[in] data The serialized data used for deserialization.
	LUNA_RUNTIME_API RV deserialize(typeinfo_t type, void* inst, const Variant& data);

	//! Deserializes one value of type `_Ty`.
	//! @param[in] inst The instance data.
	//! @param[in] data The serialized data used for deserialization.
	template <typename _Ty>
	inline RV deserialize(_Ty& inst, const Variant& data)
	{
		return deserialize(typeof<_Ty>(), &inst, data);
	}

	//! @}
}