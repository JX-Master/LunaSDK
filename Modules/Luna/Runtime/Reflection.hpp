/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TypeInfo.hpp
* @author JXMaster
* @date 2022/5/14
* @brief APIs to fetch type informations from `typeinfo_t`.
*/
#pragma once
#include "TypeInfo.hpp"
#include "Variant.hpp"

namespace Luna
{
	// Basic type informations.

	LUNA_RUNTIME_API bool is_primitive_type(typeinfo_t type);
	LUNA_RUNTIME_API bool is_struct_type(typeinfo_t type);
	LUNA_RUNTIME_API bool is_enum_type(typeinfo_t type);
	LUNA_RUNTIME_API bool is_generic_struct_type(typeinfo_t type);
	LUNA_RUNTIME_API bool is_generic_struct_instanced_type(typeinfo_t type);

	LUNA_RUNTIME_API typeinfo_t get_type_by_name(const Name& name, const Name& alias = Name());
	LUNA_RUNTIME_API Name get_type_name(typeinfo_t type, Name* alias = nullptr);
	LUNA_RUNTIME_API Guid get_type_guid(typeinfo_t type);
	LUNA_RUNTIME_API usize get_type_size(typeinfo_t type);
	LUNA_RUNTIME_API usize get_type_alignment(typeinfo_t type);
	LUNA_RUNTIME_API void* get_type_private_data(typeinfo_t type, const Guid& data_guid);
	LUNA_RUNTIME_API void* set_type_private_data(typeinfo_t type, const Guid& data_guid, usize data_size, usize data_alignment = 0, void(*data_dtor)(void*) = nullptr);

	LUNA_RUNTIME_API bool is_type_trivially_constructable(typeinfo_t type);
	LUNA_RUNTIME_API bool is_type_trivially_destructable(typeinfo_t type);
	LUNA_RUNTIME_API bool is_type_trivially_copy_constructable(typeinfo_t type);
	LUNA_RUNTIME_API bool is_type_trivially_move_constructable(typeinfo_t type);
	LUNA_RUNTIME_API bool is_type_trivially_copy_assignable(typeinfo_t type);
	LUNA_RUNTIME_API bool is_type_trivially_move_assignable(typeinfo_t type);
	LUNA_RUNTIME_API bool is_type_trivially_relocatable(typeinfo_t type);

	// Basic type operations using reflection.

	LUNA_RUNTIME_API void construct_type(typeinfo_t type, void* data);
	LUNA_RUNTIME_API void construct_type_range(typeinfo_t type, void* data, usize count);
	LUNA_RUNTIME_API void destruct_type(typeinfo_t type, void* data);
	LUNA_RUNTIME_API void destruct_type_range(typeinfo_t type, void* data, usize count);
	LUNA_RUNTIME_API void copy_construct_type(typeinfo_t type, void* dst, void* src);
	LUNA_RUNTIME_API void copy_construct_type_range(typeinfo_t type, void* dst, void* src, usize count);
	LUNA_RUNTIME_API void move_construct_type(typeinfo_t type, void* dst, void* src);
	LUNA_RUNTIME_API void move_construct_type_range(typeinfo_t type, void* dst, void* src, usize count);
	LUNA_RUNTIME_API void copy_assign_type(typeinfo_t type, void* dst, void* src);
	LUNA_RUNTIME_API void copy_assign_type_range(typeinfo_t type, void* dst, void* src, usize count);
	LUNA_RUNTIME_API void move_assign_type(typeinfo_t type, void* dst, void* src);
	LUNA_RUNTIME_API void move_assign_type_range(typeinfo_t type, void* dst, void* src, usize count);
	LUNA_RUNTIME_API void relocate_type(typeinfo_t type, void* dst, void* src);
	LUNA_RUNTIME_API void relocate_type_range(typeinfo_t type, void* dst, void* src, usize count);

	// Equal to 

	using equal_to_func_t = bool(typeinfo_t type, const void* lhs, const void* rhs);

	LUNA_RUNTIME_API bool is_type_equatable(typeinfo_t type);
	LUNA_RUNTIME_API void set_equatable(typeinfo_t type, equal_to_func_t* func);
	LUNA_RUNTIME_API bool equal_to_type(typeinfo_t type, const void* lhs, const void* rhs);

	// Hashing

	using hash_func_t = usize(typeinfo_t type, const void* inst);

	LUNA_RUNTIME_API bool is_type_hashable(typeinfo_t type);
	LUNA_RUNTIME_API void set_hashable(typeinfo_t type, hash_func_t* func);
	LUNA_RUNTIME_API usize hash_type(typeinfo_t type, const void* inst);

	// Type and property attributes.

	LUNA_RUNTIME_API void set_type_attribute(typeinfo_t type, const Name& name, const Variant& value = Variant());
	LUNA_RUNTIME_API void remove_type_attribute(typeinfo_t type, const Name& name);
	LUNA_RUNTIME_API bool check_type_attribute(typeinfo_t type, const Name& name);
	LUNA_RUNTIME_API Variant get_type_attribute(typeinfo_t type, const Name& name);
	LUNA_RUNTIME_API Vector<Name> get_type_attributes(typeinfo_t type);

	LUNA_RUNTIME_API void set_property_attribute(typeinfo_t type, const Name& property, const Name& name, const Variant& value = Variant());
	LUNA_RUNTIME_API void remove_property_attribute(typeinfo_t type, const Name& property, const Name& name);
	LUNA_RUNTIME_API bool check_property_attribute(typeinfo_t type, const Name& property, const Name& name);
	LUNA_RUNTIME_API Variant get_property_attribute(typeinfo_t type, const Name& property, const Name& name);
	LUNA_RUNTIME_API Vector<Name> get_property_attributes(typeinfo_t type, const Name& property);

	// Structure and enumeration type registration.

	struct StructurePropertyDesc
	{
		Name name;
		typeinfo_t type;
		usize offset;
		StructurePropertyDesc() = default;
		StructurePropertyDesc(const Name& name, typeinfo_t type, usize offset) :
			name(name),
			type(type),
			offset(offset) {}
	};
	struct EnumerationOptionDesc
	{
		Name name;
		i64 value;
		EnumerationOptionDesc() = default;
		EnumerationOptionDesc(const Name& name, i64 value) :
			name(name),
			value(value) {}
	};

	using structure_ctor_t = void(typeinfo_t type, void* inst);
	using structure_dtor_t = void(typeinfo_t type, void* inst);
	using structure_copy_ctor_t = void(typeinfo_t type, void* dst, void* src);
	using structure_move_ctor_t = void(typeinfo_t type, void* dst, void* src);
	using structure_copy_assign_t = void(typeinfo_t type, void* dst, void* src);
	using structure_move_assign_t = void(typeinfo_t type, void* dst, void* src);

	struct StructureTypeDesc
	{
		Guid guid;
		Name name;
		Name alias;
		//! The size of the structure type, this should include the size for the base type of this type.
		usize size;
		//! The alignment of the structure type.
		usize alignment;
		//! The base type of this structure type.
		typeinfo_t base_type = nullptr;
		//! The constructor function for this type. If `nullptr`, the default constructor will
		//! be used.
		structure_ctor_t* ctor = nullptr;
		//! The destructor function for this type. If `nullptr`, the default destructor will be used.
		structure_dtor_t* dtor = nullptr;
		//! The copy constructor for this type. If `nullptr`, the default copy constructor will
		//! be used.
		structure_copy_ctor_t* copy_ctor = nullptr;
		//! The move constructor for this type. If `nullptr`, the default move constructor will be used.
		structure_move_ctor_t* move_ctor = nullptr;
		//! The copy assignment operator for this type. If `nullptr`, the default copy assignment operator will be used.
		structure_copy_assign_t* copy_assign = nullptr;
		//! The mvoe assignment operator for this type. If `nullptr`, the default move assignment operator will be used.
		structure_move_assign_t* move_assign = nullptr;
		//! The properties of this structure type.
		Vector<StructurePropertyDesc> properties;
		//! Whether this structure is trivially relocatable.
		bool trivially_relocatable = true;
	};

	struct GenericStructureInstantiateInfo
	{
		//! The size of the structure type, this should include the size for the base type of this type.
		usize size;
		//! The alignment of the structure type.
		usize alignment;
		//! The base type of this structure type.
		typeinfo_t base_type = nullptr;
		//! The constructor function for this type. If `nullptr`, the default constructor will
		//! be used.
		structure_ctor_t* ctor = nullptr;
		//! The destructor function for this type. If `nullptr`, the default destructor will be used.
		structure_dtor_t* dtor = nullptr;
		//! The copy constructor for this type. If `nullptr`, the default copy constructor will
		//! be used.
		structure_copy_ctor_t* copy_ctor = nullptr;
		//! The move constructor for this type. If `nullptr`, the default move constructor will be used.
		structure_move_ctor_t* move_ctor = nullptr;
		//! The copy assignment operator for this type. If `nullptr`, the default copy assignment operator will be used.
		structure_copy_assign_t* copy_assign = nullptr;
		//! The mvoe assignment operator for this type. If `nullptr`, the default move assignment operator will be used.
		structure_move_assign_t* move_assign = nullptr;
		//! The properties of this structure type.
		Vector<StructurePropertyDesc> properties;
		//! Whether this structure is trivially relocatable.
		bool trivially_relocatable = true;
	};

	using generic_structure_instantiate_t = GenericStructureInstantiateInfo(typeinfo_t generic_type, const typeinfo_t* generic_arguments, usize num_generic_arguments);

	struct GenericStructureTypeDesc
	{
		Guid guid;
		Name name;
		Name alias;
		//! The names for every generic parameter.
		Vector<Name> generic_parameter_names;
		//! Whether this type suports variable number of generic arguments.
		//! If this is `true`, the user may specify zero, one or more generic arguments after arguments specified in `generic_argument_names`.
		bool variable_generic_parameters;
		//! The function used to create generic instants for this generic type.
		generic_structure_instantiate_t* instantiate;
	};

	struct EnumerationTypeDesc
	{
		Guid guid;
		Name name;
		Name alias;
		typeinfo_t underlying_type;
		Vector<EnumerationOptionDesc> options;
		bool multienum;
	};

	LUNA_RUNTIME_API typeinfo_t register_struct_type(const StructureTypeDesc& desc);
	LUNA_RUNTIME_API typeinfo_t register_generic_struct_type(const GenericStructureTypeDesc& desc);
	LUNA_RUNTIME_API typeinfo_t register_enum_type(const EnumerationTypeDesc& desc);

	LUNA_RUNTIME_API usize count_struct_properties(typeinfo_t type);
	LUNA_RUNTIME_API StructurePropertyDesc get_struct_property(typeinfo_t type, usize index);

	//! Returns the base type of the specified type.
	//! Returns `nullptr` if the specified type is not a structure type, or if the type
	//! does not have a base type.
	LUNA_RUNTIME_API typeinfo_t get_base_type(typeinfo_t type);
	LUNA_RUNTIME_API usize count_enum_options(typeinfo_t type);
	LUNA_RUNTIME_API EnumerationOptionDesc get_enum_option(typeinfo_t type, usize index);
	LUNA_RUNTIME_API typeinfo_t get_enum_underlying_type(typeinfo_t type);
	LUNA_RUNTIME_API bool is_multienum_type(typeinfo_t type);

	//! Extracts the mapped value of the enumeration, regardless of the underlying type of the enumeration.
	LUNA_RUNTIME_API i64 get_enum_instance_value(typeinfo_t type, const void* data);
	LUNA_RUNTIME_API void set_enum_instance_value(typeinfo_t type, void* data, i64 value);

	LUNA_RUNTIME_API typeinfo_t get_struct_generic_type(typeinfo_t type);
	LUNA_RUNTIME_API usize count_struct_generic_arguments(typeinfo_t type);
	LUNA_RUNTIME_API typeinfo_t get_struct_generic_argument(typeinfo_t type, usize index);
	LUNA_RUNTIME_API usize count_struct_generic_parameters(typeinfo_t type);
	LUNA_RUNTIME_API Name get_struct_generic_parameter_name(typeinfo_t type, usize index);

	template <typename _Ty>
	inline bool default_equal_to(typeinfo_t type, const void* lhs, const void* rhs)
	{
		return equal_to<_Ty>()(*((const _Ty*)lhs), *((const _Ty*)rhs));
	}
	template <typename _Ty>
	inline usize default_hash(typeinfo_t type, const void* inst)
	{
		return hash<_Ty>()(*((const _Ty*)inst));
	}
	template <typename _Ty>
	inline void default_ctor(typeinfo_t type, void* inst)
	{
		value_construct((_Ty*)inst);
	}
	template <typename _Ty>
	inline void default_dtor(typeinfo_t type, void* inst)
	{
		((_Ty*)inst)->~_Ty();
	}
	template <typename _Ty>
	inline void default_copy_ctor(typeinfo_t type, void* dst, void* src)
	{
		copy_construct((_Ty*)dst, (_Ty*)src);
	}
	template <typename _Ty>
	inline void default_move_ctor(typeinfo_t type, void* dst, void* src)
	{
		move_construct((_Ty*)dst, (_Ty*)src);
	}
	template <typename _Ty>
	inline void default_copy_assign(typeinfo_t type, void* dst, void* src)
	{
		copy_assign((_Ty*)dst, (_Ty*)src);
	}
	template <typename _Ty>
	inline void default_move_assign(typeinfo_t type, void* dst, void* src)
	{
		move_assign((_Ty*)dst, (_Ty*)src);
	}

	template <typename _Ty>
	typeinfo_t register_struct_type(InitializerList<StructurePropertyDesc> properties, typeinfo_t base_type = nullptr)
	{
		StructureTypeDesc desc;
		desc.guid = _Ty::__guid;
		desc.name = _Ty::__name;
		desc.alias = Name();
		desc.base_type = base_type;
		desc.size = sizeof(_Ty);
		desc.alignment = alignof(_Ty);
		desc.ctor = is_trivially_constructible_v<_Ty> ? nullptr : default_ctor<_Ty>;
		desc.dtor = is_trivially_destructible_v<_Ty> ? nullptr : default_dtor<_Ty>;
		desc.copy_ctor = is_trivially_copy_constructible_v<_Ty> ? nullptr : default_copy_ctor<_Ty>;
		desc.move_ctor = is_trivially_move_constructible_v<_Ty> ? nullptr : default_move_ctor<_Ty>;
		desc.copy_assign = is_trivially_copy_assignable_v<_Ty> ? nullptr : default_copy_assign<_Ty>;
		desc.move_assign = is_trivially_move_assignable_v<_Ty> ? nullptr : default_move_assign<_Ty>;
		desc.properties = properties;
		desc.trivially_relocatable = is_trivially_relocatable_v<_Ty>;
		return register_struct_type(desc);
	}

	template <typename _Ty>
	typeinfo_t register_enum_type(InitializerList<EnumerationOptionDesc> options, bool multienum = false)
	{
		EnumerationTypeDesc desc;
		desc.guid = EnumTypeInfo<_Ty>::__guid;
		desc.name = EnumTypeInfo<_Ty>::__name;
		desc.alias = Name();
		desc.underlying_type = typeof<underlying_type_t<_Ty>>();
		desc.options = options;
		desc.multienum = multienum;
		return register_enum_type(desc);
	}
}