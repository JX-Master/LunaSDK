/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TypeInfo.hpp
* @author JXMaster
* @date 2022/5/14
*/
#pragma once
#include "../TypeInfo.hpp"
#include "../UniquePtr.hpp"
#include "../SpinLock.hpp"

namespace Luna
{
	enum class TypeKind : u8
	{
		primitive,						// Primitive types.
		structure,						// Collections of properties with different names and types.
		enumeration,					// Collections of options mapped to different integers.
		generic_structure,				// Generic structure types.
		generic_structure_instanced,	// Generic structure instance types.
	};
	struct TypeInfoPrivateData
	{
		Guid guid;
		void(*dtor)(void*);
		void* data;
		usize alignment;
	};
	struct TypeInfo
	{
		TypeKind kind;
		Vector<TypeInfoPrivateData> private_data;
		Vector<Pair<Name, Variant>> attributes;
		virtual ~TypeInfo();
	};
	struct NamedTypeInfo : TypeInfo
	{
		Guid guid;
		Name name;
		Name alias;
	};
	struct PrimitiveTypeInfo : public NamedTypeInfo
	{
		usize size;
		usize alignment;
	};
	struct StructureProperty
	{
		Vector<Pair<Name, Variant>> attributes;
	};
	struct StructureTypeInfo : public NamedTypeInfo
	{
		usize size;
		usize alignment;
		TypeInfo* base_type;
		structure_ctor_t* ctor;
		structure_dtor_t* dtor;
		structure_copy_ctor_t* copy_ctor;
		structure_move_ctor_t* move_ctor;
		structure_copy_assign_t* copy_assign;
		structure_move_assign_t* move_assign;
		Array<StructurePropertyDesc> property_descs;
		Array<StructureProperty> properties;
		bool trivially_relocatable;
	};
	struct EnumerationTypeInfo : public NamedTypeInfo
	{
		PrimitiveTypeInfo* underlying_type;
		bool multienum;
		Array<EnumerationOptionDesc> options;
	};
	struct GenericStructureInstancedTypeInfo;
	struct GenericStructureTypeInfo : public NamedTypeInfo
	{
		Array<Name> generic_parameter_names;
		bool variable_generic_parameters;
		generic_structure_instantiate_t* instantiate;
		Vector<GenericStructureInstancedTypeInfo*> generic_instanced_types;
	};
	struct GenericStructureInstancedTypeInfo : public TypeInfo
	{
		GenericStructureTypeInfo* generic_type;
		Array<typeinfo_t> generic_arguments;
		usize size;
		usize alignment;
		TypeInfo* base_type;
		structure_ctor_t* ctor;
		structure_dtor_t* dtor;
		structure_copy_ctor_t* copy_ctor;
		structure_move_ctor_t* move_ctor;
		structure_copy_assign_t* copy_assign;
		structure_move_assign_t* move_assign;
		Array<StructurePropertyDesc> property_descs;
		Array<StructureProperty> properties;
		bool trivially_relocatable;
	};
	void type_registry_init();
	void type_registry_close();
}