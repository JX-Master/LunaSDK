/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TypeInfo.cpp
* @author JXMaster
* @date 2022/5/14
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "TypeInfo.hpp"
#include "../UnorderedMultiMap.hpp"
#include "../HashMap.hpp"
#include "OS.hpp"

namespace Luna
{
	Vector<UniquePtr<TypeInfo>> g_type_registry;
	opaque_t g_type_registry_lock;

	UnorderedMultiMap<Name, NamedTypeInfo*> g_type_name_map;
	HashMap<Guid, NamedTypeInfo*> g_type_guid_map;

	static typeinfo_t g_void_type;
	static typeinfo_t g_u8_type;
	static typeinfo_t g_i8_type;
	static typeinfo_t g_u16_type;
	static typeinfo_t g_i16_type;
	static typeinfo_t g_u32_type;
	static typeinfo_t g_i32_type;
	static typeinfo_t g_u64_type;
	static typeinfo_t g_i64_type;
	static typeinfo_t g_usize_type;
	static typeinfo_t g_isize_type;
	static typeinfo_t g_f32_type;
	static typeinfo_t g_f64_type;
	static typeinfo_t g_c8_type;
	static typeinfo_t g_c16_type;
	static typeinfo_t g_c32_type;
	static typeinfo_t g_boolean_type;

	LUNA_RUNTIME_API typeinfo_t void_type() { return g_void_type; }
	LUNA_RUNTIME_API typeinfo_t u8_type() { return g_u8_type; }
	LUNA_RUNTIME_API typeinfo_t i8_type() { return g_i8_type; }
	LUNA_RUNTIME_API typeinfo_t u16_type() { return g_u16_type; }
	LUNA_RUNTIME_API typeinfo_t i16_type() { return g_i16_type; }
	LUNA_RUNTIME_API typeinfo_t u32_type() { return g_u32_type; }
	LUNA_RUNTIME_API typeinfo_t i32_type() { return g_i32_type; }
	LUNA_RUNTIME_API typeinfo_t u64_type() { return g_u64_type; }
	LUNA_RUNTIME_API typeinfo_t i64_type() { return g_i64_type; }
	LUNA_RUNTIME_API typeinfo_t usize_type() { return g_usize_type; }
	LUNA_RUNTIME_API typeinfo_t isize_type() { return g_isize_type; }
	LUNA_RUNTIME_API typeinfo_t f32_type() { return g_f32_type; }
	LUNA_RUNTIME_API typeinfo_t f64_type() { return g_f64_type; }
	LUNA_RUNTIME_API typeinfo_t c8_type() { return g_c8_type; }
	LUNA_RUNTIME_API typeinfo_t c16_type() { return g_c16_type; }
	LUNA_RUNTIME_API typeinfo_t c32_type() { return g_c32_type; }
	LUNA_RUNTIME_API typeinfo_t boolean_type() { return g_boolean_type; }

	TypeInfo::~TypeInfo()
	{
		for (auto& i : private_data)
		{
			if (i.dtor) i.dtor(i.data);
			memfree(i.data, i.alignment);
		}
	}

	inline typeinfo_t add_primitive_typeinfo(const Name& name, const Guid& guid, usize size, usize alignment)
	{
		UniquePtr<TypeInfo> ti(memnew<PrimitiveTypeInfo>());
		PrimitiveTypeInfo* t = (PrimitiveTypeInfo*)ti.get();
		t->kind = TypeKind::primitive;
		t->name = name;
		t->guid = guid;
		t->size = size;
		t->alignment = alignment;
		g_type_registry.push_back(move(ti));
		g_type_name_map.insert(make_pair(name, t));
		g_type_guid_map.insert(make_pair(guid, t));
		return (typeinfo_t)t;
	}

	void type_registry_init()
	{
		g_type_registry_lock = OS::new_mutex();
		g_void_type = add_primitive_typeinfo("void", Guid("{3A153D8F-8C16-4D68-9743-C8FC675BF5E4}"), 0, 0);
		g_u8_type = add_primitive_typeinfo("u8", Guid("{23A6E98D-BB1A-469D-99D2-D2915CBAACBA}"), sizeof(u8), alignof(u8));
		g_i8_type = add_primitive_typeinfo("i8", Guid("{2624AF5D-B874-4E8F-898D-2A17D875EB9A}"), sizeof(i8), alignof(i8));
		g_u16_type = add_primitive_typeinfo("u16", Guid("{7815DB06-0230-498E-99F8-C64FEBDC5F3D}"), sizeof(u16), alignof(u16));
		g_i16_type = add_primitive_typeinfo("i16", Guid("{5CA689DA-0AE7-43FF-AA73-DF8A69EF7D69}"), sizeof(i16), alignof(i16));
		g_u32_type = add_primitive_typeinfo("u32", Guid("{281A0842-4B6B-45A5-8FBD-2F4867DD3874}"), sizeof(u32), alignof(u32));
		g_i32_type = add_primitive_typeinfo("i32", Guid("{8E239834-A603-4655-A0D7-8DC2B3BE1ABA}"), sizeof(i32), alignof(i32));
		g_u64_type = add_primitive_typeinfo("u64", Guid("{94A1106E-982B-444C-A222-DD94A2FF57B9}"), sizeof(u64), alignof(u64));
		g_i64_type = add_primitive_typeinfo("i64", Guid("{86ABAB84-3192-471A-AF20-C4FD97097F4B}"), sizeof(i64), alignof(i64));
		g_usize_type = add_primitive_typeinfo("usize", Guid("{6EC21E4F-56ED-4466-8D84-C9587F3F57FF}"), sizeof(usize), alignof(usize));
		g_isize_type = add_primitive_typeinfo("isize", Guid("{6984AC46-33AD-47CE-A491-2982D666DB90}"), sizeof(isize), alignof(isize));
		g_f32_type = add_primitive_typeinfo("f32", Guid("{EE2DD25C-F3F7-4198-805D-77B1980F90E7}"), sizeof(f32), alignof(f32));
		g_f64_type = add_primitive_typeinfo("f64", Guid("{29CCD22F-A234-45D2-B880-C99AF6ECF2ED}"), sizeof(f64), alignof(f64));
		g_c8_type = add_primitive_typeinfo("c8", Guid("{7A104397-1F4C-491D-8FD6-3D9D46B34C57}"), sizeof(c8), alignof(c8));
		g_c16_type = add_primitive_typeinfo("c16", Guid("{8ADABDAB-8503-4D5B-A20C-884A028B3E9F}"), sizeof(c16), alignof(c16));
		g_c32_type = add_primitive_typeinfo("c32", Guid("{9A5F29BB-84CC-49AB-9FF6-7022E9DFD939}"), sizeof(c32), alignof(c32));
		g_boolean_type = add_primitive_typeinfo("bool", Guid("{237D17F7-E1BA-401B-AE38-C75B04F53DB4}"), sizeof(bool), alignof(bool));
	}
	void type_registry_close()
	{
		g_type_registry.clear();
		g_type_registry.shrink_to_fit();
		g_type_name_map.clear();
		g_type_guid_map.clear();
		g_type_guid_map.shrink_to_fit();
		OS::delete_mutex(g_type_registry_lock);
	}
	static void structure_default_construct(typeinfo_t type, void* data)
	{
		StructureTypeInfo* t = (StructureTypeInfo*)type;
		// construct every field of the structure.
		for (auto& i : t->property_descs)
		{
			void* dst = (void*)((usize)data + i.offset);
			construct_type(i.type, dst);
		}
	}
	static void structure_default_destruct(typeinfo_t type, void* data)
	{
		StructureTypeInfo* t = (StructureTypeInfo*)type;
		// construct every field of the structure.
		for (auto& i : t->property_descs)
		{
			void* dst = (void*)((usize)data + i.offset);
			destruct_type(i.type, dst);
		}
	}
	static void structure_default_copy_construct(typeinfo_t type, void* dst, void* src)
	{
		StructureTypeInfo* t = (StructureTypeInfo*)type;
		// construct every field of the structure.
		for (auto& i : t->property_descs)
		{
			void* dst_property = (void*)((usize)dst + i.offset);
			void* src_property = (void*)((usize)src + i.offset);
			copy_construct_type(i.type, dst_property, src_property);
		}
	}
	static void structure_default_move_construct(typeinfo_t type, void* dst, void* src)
	{
		StructureTypeInfo* t = (StructureTypeInfo*)type;
		// construct every field of the structure.
		for (auto& i : t->property_descs)
		{
			void* dst_property = (void*)((usize)dst + i.offset);
			void* src_property = (void*)((usize)src + i.offset);
			move_construct_type(i.type, dst_property, src_property);
		}
	}
	static void structure_default_copy_assign(typeinfo_t type, void* dst, void* src)
	{
		StructureTypeInfo* t = (StructureTypeInfo*)type;
		// construct every field of the structure.
		for (auto& i : t->property_descs)
		{
			void* dst_property = (void*)((usize)dst + i.offset);
			void* src_property = (void*)((usize)src + i.offset);
			copy_assign_type(i.type, dst_property, src_property);
		}
	}
	static void structure_default_move_assign(typeinfo_t type, void* dst, void* src)
	{
		StructureTypeInfo* t = (StructureTypeInfo*)type;
		// construct every field of the structure.
		for (auto& i : t->property_descs)
		{
			void* dst_property = (void*)((usize)dst + i.offset);
			void* src_property = (void*)((usize)src + i.offset);
			move_assign_type(i.type, dst_property, src_property);
		}
	}
	LUNA_RUNTIME_API bool is_primitive_type(typeinfo_t type)
	{
		return ((TypeInfo*)type)->kind == TypeKind::primitive;
	}
	LUNA_RUNTIME_API bool is_struct_type(typeinfo_t type)
	{
		auto kind = ((TypeInfo*)type)->kind;
		return kind == TypeKind::structure ||
			kind == TypeKind::generic_structure ||
			kind == TypeKind::generic_structure_instanced;
	}
	LUNA_RUNTIME_API bool is_enum_type(typeinfo_t type)
	{
		return ((TypeInfo*)type)->kind == TypeKind::enumeration;
	}
	LUNA_RUNTIME_API bool is_generic_struct_type(typeinfo_t type)
	{
		return ((TypeInfo*)type)->kind == TypeKind::generic_structure;
	}
	LUNA_RUNTIME_API bool is_generic_struct_instanced_type(typeinfo_t type)
	{
		return ((TypeInfo*)type)->kind == TypeKind::generic_structure_instanced;
	}
	LUNA_RUNTIME_API typeinfo_t register_struct_type(const StructureTypeDesc& desc)
	{
		OSMutexGuard guard(g_type_registry_lock);
		typeinfo_t type = get_type_by_guid(desc.guid);
		if (type) return type;
		type = get_type_by_name(desc.name, desc.alias);
		if (type) return type;
		UniquePtr<TypeInfo> t(memnew<StructureTypeInfo>());
		auto st = (StructureTypeInfo*)t.get();
		st->kind = TypeKind::structure;
		st->guid = desc.guid;
		st->name = desc.name;
		st->alias = desc.alias;
		st->size = desc.size;
		st->alignment = desc.alignment;
		lucheck_msg(!desc.base_type || is_struct_type(desc.base_type), "The base type of one structure type must be a structure type.");
		st->base_type = (TypeInfo*)desc.base_type;
		st->ctor = desc.ctor;
		st->dtor = desc.dtor;
		st->copy_ctor = desc.copy_ctor;
		st->move_ctor = desc.move_ctor;
		st->copy_assign = desc.copy_assign;
		st->move_assign = desc.move_assign;
		st->trivially_relocatable = desc.trivially_relocatable;
		st->property_descs = Array<StructurePropertyDesc>(desc.properties.data(), desc.properties.size());
		st->properties = Array<StructureProperty>(desc.properties.size());
		bool use_default_ctor = false;
		bool use_default_dtor = false;
		bool use_default_copy_ctor = false;
		bool use_default_move_ctor = false;
		bool use_default_copy_assign = false;
		bool use_default_move_assign = false;
		for (auto& i : st->property_descs)
		{
			if (!is_type_trivially_constructable(i.type)) use_default_ctor = true;
			if (!is_type_trivially_destructable(i.type)) use_default_dtor = true;
			if (!is_type_trivially_copy_constructable(i.type)) use_default_copy_ctor = true;
			if (!is_type_trivially_move_constructable(i.type)) use_default_move_ctor = true;
			if (!is_type_trivially_copy_assignable(i.type)) use_default_copy_assign = true;
			if (!is_type_trivially_move_assignable(i.type)) use_default_move_assign = true;
		}
		// Adds callback for non-trivial case.
		if (!st->ctor && use_default_ctor) st->ctor = structure_default_construct;
		if (!st->dtor && use_default_dtor) st->dtor = structure_default_destruct;
		if (!st->copy_ctor && use_default_copy_ctor) st->copy_ctor = structure_default_copy_construct;
		if (!st->move_ctor && use_default_move_ctor) st->move_ctor = structure_default_move_construct;
		if (!st->copy_assign && use_default_copy_assign) st->copy_assign = structure_default_copy_assign;
		if (!st->move_assign && use_default_move_assign) st->move_assign = structure_default_move_assign;
		g_type_registry.push_back(move(t));
		g_type_name_map.insert(make_pair(st->name, (NamedTypeInfo*)st));
		g_type_guid_map.insert(make_pair(st->guid, (NamedTypeInfo*)st));
		return (typeinfo_t)st;
	}
	LUNA_RUNTIME_API typeinfo_t register_generic_struct_type(const GenericStructureTypeDesc& desc)
	{
		OSMutexGuard guard(g_type_registry_lock);
		typeinfo_t type = get_type_by_guid(desc.guid);
		if (type) return type;
		type = get_type_by_name(desc.name, desc.alias);
		if (type) return type;
		UniquePtr<TypeInfo> t(memnew<GenericStructureTypeInfo>());
		auto st = (GenericStructureTypeInfo*)t.get();
		st->kind = TypeKind::generic_structure;
		st->guid = desc.guid;
		st->name = desc.name;
		st->alias = desc.alias;
		st->generic_parameter_names.assign_n(desc.generic_parameter_names.data(), desc.generic_parameter_names.size());
		st->variable_generic_parameters = desc.variable_generic_parameters;
		st->instantiate = desc.instantiate;
		g_type_registry.push_back(move(t));
		g_type_name_map.insert(make_pair(st->name, (NamedTypeInfo*)st));
		g_type_guid_map.insert(make_pair(st->guid, (NamedTypeInfo*)st));
		return (typeinfo_t)st;
	}
	LUNA_RUNTIME_API typeinfo_t register_enum_type(const EnumerationTypeDesc& desc)
	{
		OSMutexGuard guard(g_type_registry_lock);
		typeinfo_t type = get_type_by_guid(desc.guid);
		if (type) return type;
		type = get_type_by_name(desc.name, desc.alias);
		if (type) return type;
		UniquePtr<TypeInfo> t(memnew<EnumerationTypeInfo>());
		TypeInfo* ut = (TypeInfo*)desc.underlying_type;
		lucheck_msg(ut->kind == TypeKind::primitive, "The underlying type for one enumeration must be a primitive integer type");
		EnumerationTypeInfo* et = (EnumerationTypeInfo*)t.get();
		et->kind = TypeKind::enumeration;
		et->guid = desc.guid;
		et->name = desc.name;
		et->alias = desc.alias;
		et->underlying_type = (PrimitiveTypeInfo*)ut;
		et->multienum = desc.multienum;
		et->options.assign_n(desc.options.data(), desc.options.size());
		g_type_registry.push_back(move(t));
		g_type_name_map.insert(make_pair(et->name, (NamedTypeInfo*)et));
		g_type_guid_map.insert(make_pair(et->guid, (NamedTypeInfo*)et));
		return (typeinfo_t)et;
	}

	inline bool generic_arguments_equal(const typeinfo_t* lhs_generic_arguments, usize lhs_num_generic_arguments, 
		const typeinfo_t* rhs_generic_arguments, usize rhs_num_generic_arguments)
	{
		if (lhs_num_generic_arguments != rhs_num_generic_arguments) return false;
		for (usize i = 0; i < lhs_num_generic_arguments; ++i)
		{
			if (lhs_generic_arguments[i] != rhs_generic_arguments[i]) return false;
		}
		return true;
	}

	static typeinfo_t new_instanced_type(GenericStructureTypeInfo* generic_type, Span<const typeinfo_t> generic_arguments)
	{
		UniquePtr<TypeInfo> t(memnew<GenericStructureInstancedTypeInfo>());
		auto gt = (GenericStructureInstancedTypeInfo*)t.get();
		gt->kind = TypeKind::generic_structure_instanced;
		gt->generic_type = generic_type;
		gt->generic_arguments.assign_n(generic_arguments.data(), generic_arguments.size());
		auto info = generic_type->instantiate(generic_type, generic_arguments);
		gt->size = info.size;
		gt->alignment = info.alignment;
		lucheck_msg(!info.base_type || is_struct_type(info.base_type), "The base type of one structure type must be a structure type.");
		gt->base_type = (TypeInfo*)info.base_type;
		gt->property_descs = move(info.properties);
		gt->properties = Array<StructureProperty>(gt->property_descs.size());
		gt->ctor = info.ctor;
		gt->dtor = info.dtor;
		gt->copy_ctor = info.copy_ctor;
		gt->move_ctor = info.move_ctor;
		gt->copy_assign = info.copy_assign;
		gt->move_assign = info.move_assign;
		gt->trivially_relocatable = info.trivially_relocatable;
		bool use_default_ctor = false;
		bool use_default_dtor = false;
		bool use_default_copy_ctor = false;
		bool use_default_move_ctor = false;
		bool use_default_copy_assign = false;
		bool use_default_move_assign = false;
		for (auto& i : gt->property_descs)
		{
			if (!is_type_trivially_constructable(i.type)) use_default_ctor = true;
			if (!is_type_trivially_destructable(i.type)) use_default_dtor = true;
			if (!is_type_trivially_copy_constructable(i.type)) use_default_copy_ctor = true;
			if (!is_type_trivially_move_constructable(i.type)) use_default_move_ctor = true;
			if (!is_type_trivially_copy_assignable(i.type)) use_default_copy_assign = true;
			if (!is_type_trivially_move_assignable(i.type)) use_default_move_assign = true;
		}
		// Adds callback for non-trivial case.
		if (!gt->ctor && use_default_ctor) gt->ctor = structure_default_construct;
		if (!gt->dtor && use_default_dtor) gt->dtor = structure_default_destruct;
		if (!gt->copy_ctor && use_default_copy_ctor) gt->copy_ctor = structure_default_copy_construct;
		if (!gt->move_ctor && use_default_move_ctor) gt->move_ctor = structure_default_move_construct;
		if (!gt->copy_assign && use_default_copy_assign) gt->copy_assign = structure_default_copy_assign;
		if (!gt->move_assign && use_default_move_assign) gt->move_assign = structure_default_move_assign;
		g_type_registry.push_back(move(t));
		generic_type->generic_instanced_types.push_back(gt);
		return (typeinfo_t)gt;
	}
	LUNA_RUNTIME_API typeinfo_t get_type_by_name(const Name& name, const Name& alias)
	{
		OSMutexGuard guard(g_type_registry_lock);
		auto range = g_type_name_map.equal_range(name);
		if (range.first == range.second) return nullptr;
		for (auto iter = range.first; iter != range.second; ++iter)
		{
			NamedTypeInfo* type = iter->second;
			if (type->alias != alias) continue;
			return (typeinfo_t)type;
		}
		return nullptr;
	}
	LUNA_RUNTIME_API typeinfo_t get_type_by_guid(const Guid& guid)
	{
		OSMutexGuard guard(g_type_registry_lock);
		auto iter = g_type_guid_map.find(guid);
		if (iter == g_type_guid_map.end()) return nullptr;
		return (typeinfo_t)(iter->second);
	}
	LUNA_RUNTIME_API typeinfo_t get_generic_instanced_type(typeinfo_t generic_type, Span<const typeinfo_t> generic_arguments)
	{
		OSMutexGuard guard(g_type_registry_lock);
		if (((TypeInfo*)generic_type)->kind != TypeKind::generic_structure) return nullptr;
		GenericStructureTypeInfo* st = (GenericStructureTypeInfo*)generic_type;
		for (GenericStructureInstancedTypeInfo* gt : st->generic_instanced_types)
		{
			if (generic_arguments_equal(gt->generic_arguments.data(), gt->generic_arguments.size(), generic_arguments.data(), generic_arguments.size())) return (typeinfo_t)gt;
		}
		if (generic_arguments.size() == 0) return nullptr;
		// Creates a new type for generic arguments.
		return new_instanced_type(st, generic_arguments);
	}
	LUNA_RUNTIME_API Name get_type_name(typeinfo_t type, Name* alias)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: if(alias) *alias = ((PrimitiveTypeInfo*)t)->alias; return ((PrimitiveTypeInfo*)t)->name;
		case TypeKind::structure: if(alias) *alias = ((StructureTypeInfo*)t)->alias; return ((StructureTypeInfo*)t)->name;
		case TypeKind::enumeration: if(alias) *alias = ((EnumerationTypeInfo*)t)->alias; return ((EnumerationTypeInfo*)t)->underlying_type->name;
		case TypeKind::generic_structure: if(alias) *alias = ((GenericStructureTypeInfo*)t)->alias; return ((GenericStructureTypeInfo*)t)->name;
		case TypeKind::generic_structure_instanced: if(alias) *alias = ((GenericStructureInstancedTypeInfo*)t)->generic_type->alias; return ((GenericStructureInstancedTypeInfo*)t)->generic_type->name;
		default: lupanic();
		}
		return Name();
	}
	LUNA_RUNTIME_API Guid get_type_guid(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return ((PrimitiveTypeInfo*)t)->guid;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->guid;
		case TypeKind::enumeration: return ((EnumerationTypeInfo*)t)->underlying_type->guid;
		case TypeKind::generic_structure: return ((GenericStructureTypeInfo*)t)->guid;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->generic_type->guid;
		default: lupanic();
		}
		return Guid(0, 0);
	}
	LUNA_RUNTIME_API usize get_type_size(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return ((PrimitiveTypeInfo*)t)->size;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->size;
		case TypeKind::enumeration: return ((EnumerationTypeInfo*)t)->underlying_type->size;
		case TypeKind::generic_structure: return 0;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->size;
		default: lupanic();
		}
		return 0;
	}
	LUNA_RUNTIME_API usize get_type_alignment(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return ((PrimitiveTypeInfo*)t)->alignment;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->alignment;
		case TypeKind::enumeration: return ((EnumerationTypeInfo*)t)->underlying_type->alignment;
		case TypeKind::generic_structure: return 0;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->alignment;
		default: lupanic();
		}
		return 0;
	}
	LUNA_RUNTIME_API typeinfo_t get_struct_generic_type(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->generic_type;
		case TypeKind::primitive: return nullptr;
		case TypeKind::structure: return nullptr;
		case TypeKind::enumeration: return nullptr;
		case TypeKind::generic_structure: return nullptr;
		default: lupanic();
		}
		return nullptr;
	}
	LUNA_RUNTIME_API Span<const typeinfo_t> get_struct_generic_arguments(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::generic_structure_instanced:
		{
			GenericStructureInstancedTypeInfo* src = ((GenericStructureInstancedTypeInfo*)t);
			return Span<const typeinfo_t>(src->generic_arguments.data(), src->generic_arguments.size());
		}
		case TypeKind::primitive: return Span<const typeinfo_t>();
		case TypeKind::structure: return Span<const typeinfo_t>();
		case TypeKind::enumeration: return Span<const typeinfo_t>();
		case TypeKind::generic_structure: return Span<const typeinfo_t>();
		default: lupanic();
		}
		return Span<const typeinfo_t>();
	}
	LUNA_RUNTIME_API usize count_struct_generic_parameters(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->generic_type->generic_parameter_names.size();
		case TypeKind::primitive: return 0;
		case TypeKind::structure: return 0;
		case TypeKind::enumeration: return 0;
		case TypeKind::generic_structure: return ((GenericStructureTypeInfo*)t)->generic_parameter_names.size();
		default: lupanic();
		}
		return 0;
	}
	LUNA_RUNTIME_API Span<const Name> get_struct_generic_parameter_names(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::generic_structure_instanced:
		{
			auto src = ((GenericStructureInstancedTypeInfo*)t);
			return Span<const Name>(src->generic_type->generic_parameter_names.data(), src->generic_type->generic_parameter_names.size());
		}
		case TypeKind::primitive: return Span<const Name>();
		case TypeKind::structure: return Span<const Name>();
		case TypeKind::enumeration: return Span<const Name>();
		case TypeKind::generic_structure:
		{
			auto src = ((GenericStructureTypeInfo*)t);
			return Span<const Name>(src->generic_parameter_names.data(), src->generic_parameter_names.size());
		} 
		default: lupanic();
		}
		return Span<const Name>();
	}
	LUNA_RUNTIME_API bool is_type_trivially_constructable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->ctor == nullptr;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->ctor == nullptr;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API bool is_type_trivially_destructable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->dtor == nullptr;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->dtor == nullptr;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API bool is_type_trivially_copy_constructable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->copy_ctor == nullptr;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->copy_ctor == nullptr;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API bool is_type_trivially_move_constructable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->move_ctor == nullptr;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->move_ctor == nullptr;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API bool is_type_trivially_copy_assignable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->copy_assign == nullptr;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->copy_assign == nullptr;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API bool is_type_trivially_move_assignable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->move_assign == nullptr;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->move_assign == nullptr;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API bool is_type_trivially_relocatable(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: return true;
		case TypeKind::structure: return ((StructureTypeInfo*)t)->trivially_relocatable;
		case TypeKind::enumeration: return true;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->trivially_relocatable;
		default: lupanic();
		}
		return true;
	}
	LUNA_RUNTIME_API void* get_type_private_data(typeinfo_t type, const Guid& data_guid)
	{
		TypeInfo* t = (TypeInfo*)type;
		for (auto& i : t->private_data)
		{
			if (i.guid == data_guid)
			{
				return i.data;
			}
		}
		if (t->kind == TypeKind::generic_structure_instanced)
		{
			return get_type_private_data(((GenericStructureInstancedTypeInfo*)t)->generic_type, data_guid);
		}
		return nullptr;
	}
	LUNA_RUNTIME_API void* set_type_private_data(typeinfo_t type, const Guid& data_guid, usize data_size, usize data_alignment, void(*data_dtor)(void*))
	{
		TypeInfo* t = (TypeInfo*)type;
		for (auto iter = t->private_data.begin(); iter != t->private_data.end(); ++iter)
		{
			if (iter->guid == data_guid)
			{
				if (iter->dtor) iter->dtor(iter->data);
				memfree(iter->data, iter->alignment);
				if (data_size)
				{
					iter->data = memalloc(data_size, data_alignment);
					iter->dtor = data_dtor;
					iter->alignment = data_alignment;
					return iter->data;
				}
				else
				{
					t->private_data.erase(iter);
					return nullptr;
				}
			}
		}
		if (data_size)
		{
			t->private_data.emplace_back();
			auto& data = t->private_data.back();
			data.guid = data_guid;
			data.data = memalloc(data_size, data_alignment);
			data.dtor = data_dtor;
			data.alignment = data_alignment;
			return data.data;
		}
		return nullptr;
	}
	LUNA_RUNTIME_API void construct_type(typeinfo_t type, void* data)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memzero(data, ((PrimitiveTypeInfo*)t)->size); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->ctor;
			if (func) func(type, data);
			else memzero(data, ((StructureTypeInfo*)t)->size);
			break;
		}
		case TypeKind::enumeration: memzero(data, ((EnumerationTypeInfo*)t)->underlying_type->size); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->ctor;
			if (func) func(type, data);
			else memzero(data, ((GenericStructureInstancedTypeInfo*)t)->size);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot construct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void construct_type_range(typeinfo_t type, void* data, usize count)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memzero(data, ((PrimitiveTypeInfo*)t)->size * count); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->ctor;
			auto size = ((StructureTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)data + i * size));
				}
			}
			else memzero(data, size * count);
			break;
		}
		case TypeKind::enumeration: memzero(data, ((EnumerationTypeInfo*)t)->underlying_type->size * count); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->ctor;
			auto size = ((GenericStructureInstancedTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)data + i * size));
				}
			}
			else memzero(data, size * count);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot construct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void destruct_type(typeinfo_t type, void* data)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->dtor;
			if (func) func(type, data);
			break;
		}
		case TypeKind::enumeration: break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->dtor;
			if (func) func(type, data);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot destruct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void destruct_type_range(typeinfo_t type, void* data, usize count)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->dtor;
			if (func)
			{
				auto size = ((StructureTypeInfo*)t)->size;
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)data + i * size));
				}
			}
			break;
		}
		case TypeKind::enumeration: break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->dtor;
			if (func)
			{
				auto size = ((GenericStructureInstancedTypeInfo*)t)->size;
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)data + i * size));
				}
			}
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot destruct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void copy_construct_type(typeinfo_t type, void* dst, void* src)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->copy_ctor;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((StructureTypeInfo*)t)->size);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->copy_ctor;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((GenericStructureInstancedTypeInfo*)t)->size);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot copy-construct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void copy_construct_type_range(typeinfo_t type, void* dst, void* src, usize count)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size * count); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->copy_ctor;
			auto size = ((StructureTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size * count); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->copy_ctor;
			auto size = ((GenericStructureInstancedTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot copy-construct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void move_construct_type(typeinfo_t type, void* dst, void* src)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->move_ctor;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((StructureTypeInfo*)t)->size);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->move_ctor;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((GenericStructureInstancedTypeInfo*)t)->size);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot move-construct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void move_construct_type_range(typeinfo_t type, void* dst, void* src, usize count)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size * count); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->move_ctor;
			auto size = ((StructureTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size * count); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->move_ctor;
			auto size = ((GenericStructureInstancedTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot move-construct a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void copy_assign_type(typeinfo_t type, void* dst, void* src)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->copy_assign;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((StructureTypeInfo*)t)->size);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->copy_assign;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((GenericStructureInstancedTypeInfo*)t)->size);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot copy-assign a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void copy_assign_type_range(typeinfo_t type, void* dst, void* src, usize count)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size * count); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->copy_assign;
			auto size = ((StructureTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size * count); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->copy_assign;
			auto size = ((GenericStructureInstancedTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot copy-assign a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void move_assign_type(typeinfo_t type, void* dst, void* src)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->move_assign;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((StructureTypeInfo*)t)->size);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->move_assign;
			if (func) func(type, dst, src);
			else memcpy(dst, src, ((GenericStructureInstancedTypeInfo*)t)->size);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot move-assign a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void move_assign_type_range(typeinfo_t type, void* dst, void* src, usize count)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::primitive: memcpy(dst, src, ((PrimitiveTypeInfo*)t)->size * count); break;
		case TypeKind::structure:
		{
			auto func = ((StructureTypeInfo*)t)->move_assign;
			auto size = ((StructureTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::enumeration: memcpy(dst, src, ((EnumerationTypeInfo*)t)->underlying_type->size * count); break;
		case TypeKind::generic_structure_instanced:
		{
			auto func = ((GenericStructureInstancedTypeInfo*)t)->move_assign;
			auto size = ((GenericStructureInstancedTypeInfo*)t)->size;
			if (func)
			{
				for (usize i = 0; i < count; ++i)
				{
					func(type, (void*)((usize)dst + i * size), (void*)((usize)src + i * size));
				}
			}
			else memcpy(dst, src, size * count);
			break;
		}
		case TypeKind::generic_structure: lupanic_msg("Cannot move-assign a generic type."); break;
		default: lupanic();
		}
	}
	LUNA_RUNTIME_API void relocate_type(typeinfo_t type, void* dst, void* src)
	{
		if (is_type_trivially_relocatable(type))
		{
			memcpy(dst, src, get_type_size(type));
		}
		else
		{
			move_construct_type(type, dst, src);
			destruct_type(type, src);
		}
	}
	LUNA_RUNTIME_API void relocate_type_range(typeinfo_t type, void* dst, void* src, usize count)
	{
		if (is_type_trivially_relocatable(type))
		{
			memcpy(dst, src, get_type_size(type) * count);
		}
		else
		{
			move_construct_type_range(type, dst, src, count);
			destruct_type_range(type, src, count);
		}
	}
	LUNA_RUNTIME_API Span<const StructurePropertyDesc> get_struct_properties(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure:
		{
			auto& src = ((StructureTypeInfo*)t)->property_descs;
			return Span<const StructurePropertyDesc>(src.data(), src.size());
		}
		case TypeKind::generic_structure_instanced:
		{
			auto& src = ((GenericStructureInstancedTypeInfo*)t)->property_descs;
			return Span<const StructurePropertyDesc>(src.data(), src.size());
		} 
        default: break;
		}
		return Span<const StructurePropertyDesc>();
	}
	LUNA_RUNTIME_API typeinfo_t get_base_type(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure: return ((StructureTypeInfo*)t)->base_type;
		case TypeKind::generic_structure_instanced: return ((GenericStructureInstancedTypeInfo*)t)->base_type;
		default: break;
		}
		return nullptr;
	}
	LUNA_RUNTIME_API Span<const EnumerationOptionDesc> get_enum_options(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::enumeration:
		{
			auto& src = ((EnumerationTypeInfo*)t)->options;
			return Span<const EnumerationOptionDesc>(src.data(), src.size());
		}
        default: break;
		}
		return Span<const EnumerationOptionDesc>();
	}
	LUNA_RUNTIME_API typeinfo_t get_enum_underlying_type(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::enumeration: return ((EnumerationTypeInfo*)t)->underlying_type;
		default: break;
		}
		lupanic();
		return nullptr;
	}
	LUNA_RUNTIME_API bool is_multienum_type(typeinfo_t type)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::enumeration: return ((EnumerationTypeInfo*)t)->multienum;
		default: break;
		}
		return false;
	}
	LUNA_RUNTIME_API i64 get_enum_instance_value(typeinfo_t type, const void* data)
	{
		PrimitiveTypeInfo* underlying_type = (PrimitiveTypeInfo*)get_enum_underlying_type(type);
		switch (underlying_type->size)
		{
		case 1: return *((const i8*)data);
		case 2: return *((const i16*)data);
		case 4: return *((const i32*)data);
		case 8: return *((const i64*)data);
		default: lupanic(); return 0;
		}
	}
	LUNA_RUNTIME_API void set_enum_instance_value(typeinfo_t type, void* data, i64 value)
	{
		PrimitiveTypeInfo* underlying_type = (PrimitiveTypeInfo*)get_enum_underlying_type(type);
		switch (underlying_type->size)
		{
		case 1: *((i8*)data) = (i8)value; break;
		case 2: *((i16*)data) = (i16)value; break;
		case 4: *((i32*)data) = (i32)value; break;
		case 8: *((i64*)data) = (i64)value; break;
		default: lupanic();
		}
	}
	constexpr Guid equal_to_data_guid("{A04DAB44-5DDD-4A36-92F3-6E63F850EC4C}");
	LUNA_RUNTIME_API bool is_type_equatable(typeinfo_t type)
	{
		return get_type_private_data(type, equal_to_data_guid) != nullptr;
	}
	LUNA_RUNTIME_API void set_equatable(typeinfo_t type, equal_to_func_t* func)
	{
		equal_to_func_t** data = (equal_to_func_t**)set_type_private_data(type, equal_to_data_guid, sizeof(equal_to_func_t*));
		*data = func;
	}
	LUNA_RUNTIME_API bool equal_to_type(typeinfo_t type, const void* lhs, const void* rhs)
	{
		equal_to_func_t** data = (equal_to_func_t**)get_type_private_data(type, equal_to_data_guid);
		if (!data) return false;
		return (*data)(type, lhs, rhs);
	}
	constexpr Guid hash_data_guid("{1641C706-AC08-4E20-87C1-2D9954B5AF02}");
	LUNA_RUNTIME_API bool is_type_hashable(typeinfo_t type)
	{
		return get_type_private_data(type, hash_data_guid) != nullptr;
	}
	LUNA_RUNTIME_API void set_hashable(typeinfo_t type, hash_func_t* func)
	{
		hash_func_t** data = (hash_func_t**)set_type_private_data(type, hash_data_guid, sizeof(hash_func_t*));
		*data = func;
	}
	LUNA_RUNTIME_API usize hash_type(typeinfo_t type, const void* inst)
	{
		hash_func_t** data = (hash_func_t**)get_type_private_data(type, hash_data_guid);
		if (!data) return 0;
		return (*data)(type, inst);
	}
	inline void set_attribute(Vector<Pair<Name, Variant>>& attributes, const Name& name, const Variant& value)
	{
		for (auto& a : attributes)
			{
				if (a.first == name)
				{
					a.second = value; return;
				}
			}
		attributes.push_back(make_pair(name, value));
	}
	LUNA_RUNTIME_API void set_type_attribute(typeinfo_t type, const Name& name, const Variant& value)
	{
		TypeInfo* t = (TypeInfo*)type;
		set_attribute(t->attributes, name, value);
	}
	inline void remove_attribute(Vector<Pair<Name, Variant>>& attributes, const Name& name)
	{
		for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
		{
			if (iter->first == name)
			{
				attributes.erase(iter);
				return;
			}
		}
	}
	LUNA_RUNTIME_API void remove_type_attribute(typeinfo_t type, const Name& name)
	{
		TypeInfo* t = (TypeInfo*)type;
		remove_attribute(t->attributes, name);
	}
	inline bool check_attribute(Vector<Pair<Name, Variant>>& attributes, const Name& name)
	{
		for (auto& a : attributes)
		{
			if (a.first == name)
			{
				return true;
			}
		}
		return false;
	}
	LUNA_RUNTIME_API bool check_type_attribute(typeinfo_t type, const Name& name)
	{
		TypeInfo* t = (TypeInfo*)type;
		auto ret = check_attribute(t->attributes, name);
		if (ret == false && t->kind == TypeKind::generic_structure_instanced)
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			return check_attribute(t2->generic_type->attributes, name);
		}
		return ret;
	}
	inline Pair<Variant, bool> get_attribute(Vector<Pair<Name, Variant>>& attributes, const Name& name)
	{
		for (auto& a : attributes)
		{
			if (a.first == name)
			{
				return make_pair(a.second, true);
			}
		}
		return make_pair(Variant(), false);
	}
	LUNA_RUNTIME_API Variant get_type_attribute(typeinfo_t type, const Name& name)
	{
		TypeInfo* t = (TypeInfo*)type;
		auto ret = get_attribute(t->attributes, name);
		if (ret.second == false && t->kind == TypeKind::generic_structure_instanced)
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			ret = get_attribute(t2->generic_type->attributes, name);
		}
		return ret.first;
	}
	inline Vector<Name> get_attributes(Vector<Pair<Name, Variant>>& attributes)
	{
		Vector<Name> ret;
		for (auto& i : attributes)
		{
			ret.push_back(i.first);
		}
		return ret;
	}
	LUNA_RUNTIME_API Vector<Name> get_type_attributes(typeinfo_t type)
	{
		Vector<Name> ret;
		TypeInfo* t = (TypeInfo*)type;
		return get_attributes(t->attributes);
	}
	LUNA_RUNTIME_API void set_property_attribute(typeinfo_t type, const Name& property, const Name& name, const Variant& value)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure:
		{
			auto t2 = (StructureTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					set_attribute(t2->properties[i].attributes, name, value);
					break;
				}
			}
		}
		break;
		case TypeKind::generic_structure_instanced:
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					set_attribute(t2->properties[i].attributes, name, value);
					break;
				}
			}
		}
		break;
		default: lupanic_msg("Only structure types may have property attributes.");
		}
	}
	LUNA_RUNTIME_API void remove_property_attribute(typeinfo_t type, const Name& property, const Name& name)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure:
		{
			auto t2 = (StructureTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					remove_attribute(t2->properties[i].attributes, name);
					break;
				}
			}
		}
		break;
		case TypeKind::generic_structure_instanced:
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					remove_attribute(t2->properties[i].attributes, name);
					break;
				}
			}
		}
		break;
		default: lupanic_msg("Only structure types may have property attributes.");
		}
	}
	LUNA_RUNTIME_API bool check_property_attribute(typeinfo_t type, const Name& property, const Name& name)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure:
		{
			auto t2 = (StructureTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					return check_attribute(t2->properties[i].attributes, name);
				}
			}
		}
		break;
		case TypeKind::generic_structure_instanced:
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					return check_attribute(t2->properties[i].attributes, name);
				}
			}
		}
		break;
		default: lupanic_msg("Only structure types may have property attributes.");
		}
		return false;
	}
	LUNA_RUNTIME_API Variant get_property_attribute(typeinfo_t type, const Name& property, const Name& name)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure:
		{
			auto t2 = (StructureTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					return get_attribute(t2->properties[i].attributes, name).first;
				}
			}
		}
		break;
		case TypeKind::generic_structure_instanced:
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					return get_attribute(t2->properties[i].attributes, name).first;
				}
			}
		}
		break;
		default: lupanic_msg("Only structure types may have property attributes.");
		}
		return Variant();
	}
	LUNA_RUNTIME_API Vector<Name> get_property_attributes(typeinfo_t type, const Name& property)
	{
		TypeInfo* t = (TypeInfo*)type;
		switch (t->kind)
		{
		case TypeKind::structure:
		{
			auto t2 = (StructureTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					return get_attributes(t2->properties[i].attributes);
				}
			}
		}
		break;
		case TypeKind::generic_structure_instanced:
		{
			auto t2 = (GenericStructureInstancedTypeInfo*)t;
			for(usize i = 0; i < t2->properties.size(); ++i)
			{
				if (t2->property_descs[i].name == property)
				{
					return get_attributes(t2->properties[i].attributes);
				}
			}
		}
		break;
		default: lupanic_msg("Only structure types may have property attributes.");
		}
		return Vector<Name>();
	}
}
