/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BuiltTypeInfo.hpp
* @author JXMaster
* @date 2022/5/23
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../TypeInfo.hpp"
#include "../Serialization.hpp"
#include "../Object.hpp"
#include "../Path.hpp"
#include "../HashMap.hpp"
#include "../HashSet.hpp"
#include "../Math/Vector.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Color.hpp"

namespace Luna
{
	static R<Variant> serialize_guid(typeinfo_t type, const void* inst)
	{
		const Guid* guid = (const Guid*)inst;
		Variant ret(VariantType::array);
		ret.push_back(guid->low);
		ret.push_back(guid->high);
		return ret;
	}
	static RV deserialize_guid(typeinfo_t type, void* inst, const Variant& data)
	{
		Guid* guid = (Guid*)inst;
		guid->low = data[0].unum();
		guid->high = data[1].unum();
		return ok;
	}
	static R<Variant> serialize_version(typeinfo_t type, const void* inst)
	{
		const Version* v = (const Version*)inst;
		Variant ret(VariantType::array);
		ret.push_back((u64)v->major);
		ret.push_back((u64)v->minor);
		ret.push_back((u64)v->patch);
		return ret;
	}
	static RV deserialize_version(typeinfo_t type, void* inst, const Variant& data)
	{
		Version* v = (Version*)inst;
		v->major = (u32)data[0].unum();
		v->minor = (u32)data[1].unum();
		v->patch = (u32)data[2].unum();
		return ok;
	}
	static R<Variant> serialize_u8(typeinfo_t type, const void* inst) { return Variant((u64) * ((u8*)inst)); }
	static R<Variant> serialize_i8(typeinfo_t type, const void* inst) { return Variant((i64) * ((i8*)inst)); }
	static R<Variant> serialize_u16(typeinfo_t type, const void* inst) { return Variant((u64) * ((u16*)inst)); }
	static R<Variant> serialize_i16(typeinfo_t type, const void* inst) { return Variant((i64) * ((i16*)inst)); }
	static R<Variant> serialize_u32(typeinfo_t type, const void* inst) { return Variant((u64) * ((u32*)inst)); }
	static R<Variant> serialize_i32(typeinfo_t type, const void* inst) { return Variant((i64) * ((i32*)inst)); }
	static R<Variant> serialize_u64(typeinfo_t type, const void* inst) { return Variant((u64) * ((u64*)inst)); }
	static R<Variant> serialize_i64(typeinfo_t type, const void* inst) { return Variant((i64) * ((i64*)inst)); }
	static R<Variant> serialize_usize(typeinfo_t type, const void* inst) { return Variant((u64) * ((usize*)inst)); }
	static R<Variant> serialize_isize(typeinfo_t type, const void* inst) { return Variant((i64) * ((isize*)inst)); }
	static R<Variant> serialize_f32(typeinfo_t type, const void* inst) { return Variant((f64) * ((f32*)inst)); }
	static R<Variant> serialize_f64(typeinfo_t type, const void* inst) { return Variant((f64) * ((f64*)inst)); }
	static R<Variant> serialize_c8(typeinfo_t type, const void* inst) { return Variant((u64) * ((c8*)inst)); }
	static R<Variant> serialize_c16(typeinfo_t type, const void* inst) { return Variant((u64) * ((c16*)inst)); }
	static R<Variant> serialize_c32(typeinfo_t type, const void* inst) { return Variant((u64) * ((c32*)inst)); }
	static R<Variant> serialize_bool(typeinfo_t type, const void* inst) { return Variant(*((bool*)inst)); }
	static RV deserialize_u8(typeinfo_t type, void* inst, const Variant& data) { *((u8*)inst) = (u8)data.unum(); return ok; }
	static RV deserialize_i8(typeinfo_t type, void* inst, const Variant& data) { *((i8*)inst) = (i8)data.inum(); return ok; }
	static RV deserialize_u16(typeinfo_t type, void* inst, const Variant& data) { *((u16*)inst) = (u16)data.unum(); return ok; }
	static RV deserialize_i16(typeinfo_t type, void* inst, const Variant& data) { *((i16*)inst) = (i16)data.inum(); return ok; }
	static RV deserialize_u32(typeinfo_t type, void* inst, const Variant& data) { *((u32*)inst) = (u32)data.unum(); return ok; }
	static RV deserialize_i32(typeinfo_t type, void* inst, const Variant& data) { *((i32*)inst) = (i32)data.inum(); return ok; }
	static RV deserialize_u64(typeinfo_t type, void* inst, const Variant& data) { *((u64*)inst) = (u64)data.unum(); return ok; }
	static RV deserialize_i64(typeinfo_t type, void* inst, const Variant& data) { *((i64*)inst) = (i64)data.inum(); return ok; }
	static RV deserialize_usize(typeinfo_t type, void* inst, const Variant& data) { *((usize*)inst) = (usize)data.unum(); return ok; }
	static RV deserialize_isize(typeinfo_t type, void* inst, const Variant& data) { *((isize*)inst) = (isize)data.inum(); return ok; }
	static RV deserialize_f32(typeinfo_t type, void* inst, const Variant& data) { *((f32*)inst) = (f32)data.fnum(); return ok; }
	static RV deserialize_f64(typeinfo_t type, void* inst, const Variant& data) { *((f64*)inst) = (f64)data.fnum(); return ok; }
	static RV deserialize_c8(typeinfo_t type, void* inst, const Variant& data) { *((c8*)inst) = (c8)data.unum(); return ok; }
	static RV deserialize_c16(typeinfo_t type, void* inst, const Variant& data) { *((c16*)inst) = (c16)data.unum(); return ok; }
	static RV deserialize_c32(typeinfo_t type, void* inst, const Variant& data) { *((c32*)inst) = (c32)data.unum(); return ok; }
	static RV deserialize_bool(typeinfo_t type, void* inst, const Variant& data) { *((bool*)inst) = data.boolean(); return ok; }

	static R<Variant> serialize_string(typeinfo_t type, const void* inst)
	{
		const String* s = (const String*)inst;
		return Variant(*s);
	}
	static RV deserialize_string(typeinfo_t type, void* inst, const Variant& data)
	{
		String* s = (String*)inst;
		*s = data.str().c_str();
		return ok;
	}
	static R<Variant> serialize_name(typeinfo_t type, const void* inst)
	{
		const Name* s = (const Name*)inst;
		return Variant(*s);
	}
	static RV deserialize_name(typeinfo_t type, void* inst, const Variant& data)
	{
		Name* s = (Name*)inst;
		*s = data.str();
		return ok;
	}
	struct VectorData
	{
		void* m_buffer;
		usize m_size;
		usize m_capacity;

		void free_buffer(typeinfo_t element_type)
		{
			if (m_buffer)
			{
				destruct_type_range(element_type, m_buffer, m_size);
				memfree(m_buffer, get_type_alignment(element_type));
				m_buffer = nullptr;
			}
			m_size = 0;
			m_capacity = 0;
		}
		void reserve(typeinfo_t element_type, usize element_size, usize new_cap)
		{
			if (new_cap > m_capacity)
			{
				void* new_buf = memalloc(element_size * new_cap, get_type_alignment(element_type));
				if (m_buffer)
				{
					relocate_type_range(element_type, new_buf, m_buffer, m_size);
				}
				m_buffer = new_buf;
				m_capacity = new_cap;
			}
		}
	};
	static void vector_dtor(typeinfo_t type, void* inst)
	{
		VectorData* vec = (VectorData*)inst;
		typeinfo_t element_type = get_struct_generic_arguments(type)[0];
		vec->free_buffer(element_type);
	}
	static void vector_copy_ctor(typeinfo_t type, void* dest, void* src)
	{
		VectorData* dest_vec = (VectorData*)dest;
		VectorData* src_vec = (VectorData*)src;
		typeinfo_t element_type = get_struct_generic_arguments(type)[0];
		dest_vec->m_buffer = nullptr;
		dest_vec->m_size = src_vec->m_size;
		dest_vec->m_capacity = src_vec->m_size;
		if (src_vec->m_size)
		{
			usize element_size = get_type_size(element_type);
			dest_vec->m_buffer = memalloc(element_size * src_vec->m_size, get_type_alignment(element_type));
			copy_construct_type_range(element_type, dest_vec->m_buffer, src_vec->m_buffer, src_vec->m_size);
		}
	}
	static void vector_move_ctor(typeinfo_t type, void* dest, void* src)
	{
		VectorData* dest_vec = (VectorData*)dest;
		VectorData* src_vec = (VectorData*)src;
		*dest_vec = *src_vec;
		src_vec->m_buffer = nullptr;
		src_vec->m_capacity = 0;
		src_vec->m_size = 0;
	}
	static void vector_copy_assign(typeinfo_t type, void* dest, void* src)
	{
		vector_dtor(type, dest);
		vector_copy_ctor(type, dest, src);
	}
	static void vector_move_assign(typeinfo_t type, void* dest, void* src)
	{
		vector_dtor(type, dest);
		vector_move_ctor(type, dest, src);
	}
	static GenericStructureInstantiateInfo vector_instantiate(typeinfo_t base_type, Span<const typeinfo_t> generic_arguments)
	{
		GenericStructureInstantiateInfo ret;
		ret.size = sizeof(VectorData);
		ret.alignment = alignof(VectorData);
		ret.base_type = nullptr;
		ret.ctor = nullptr;
		ret.dtor = vector_dtor;
		ret.copy_ctor = vector_copy_ctor;
		ret.move_ctor = vector_move_ctor;
		ret.copy_assign = vector_copy_assign;
		ret.move_assign = vector_move_assign;
		ret.trivially_relocatable = true;
		return ret;
	}
	static R<Variant> serialize_vector(typeinfo_t type, const void* inst)
	{
		Variant ret(VariantType::array);
		lutry
		{
			VectorData* vec = (VectorData*)inst;
			typeinfo_t element_type = get_struct_generic_arguments(type)[0];
			usize element_size = get_type_size(element_type);
			for (usize i = 0; i < vec->m_size; ++i)
			{
				lulet(data, serialize(element_type, (void*)((usize)vec->m_buffer + i * element_size)));
				ret.push_back(move(data));
			}
		}
		lucatchret;
		return ret;
	}
	static RV deserialize_vector(typeinfo_t type, void* inst, const Variant& data)
	{
		if (data.type() != VariantType::array) return BasicError::bad_arguments();
		VectorData* vec = (VectorData*)inst;
		typeinfo_t element_type = get_struct_generic_arguments(type)[0];
		usize element_size = get_type_size(element_type);
		vector_dtor(type, vec);
		vec->reserve(element_type, element_size, data.size());
		lutry
		{
			for (usize i = 0; i < data.size(); ++i)
			{
				void* dst = (void*)((usize)vec->m_buffer + i * element_size);
				construct_type(element_type, dst);
				luexp(deserialize(element_type, dst, data[i]));
			}
		}
		lucatchret;
		vec->m_size = data.size();
		return ok;
	}
	static R<Variant> serialize_path(typeinfo_t type, const void* inst)
	{
		const Path* p = (const Path*)inst;
		return Variant(Name(p->encode()));
	}
	static RV deserialize_path(typeinfo_t type, void* inst, const Variant& data)
	{
		Path* p = (Path*)inst;
		p->assign(data.c_str());
		return ok;
	}
	static GenericStructureInstantiateInfo pair_instantiate(typeinfo_t base_type, Span<const typeinfo_t> generic_arguments)
	{
		typeinfo_t first = generic_arguments[0];
		typeinfo_t second = generic_arguments[1];
		MemoryLayoutMember members[2] = {
			MemoryLayoutMember(get_type_size(first), get_type_alignment(first)),
			MemoryLayoutMember(get_type_size(second), get_type_alignment(second))
		};
		GenericStructureInstantiateInfo ret;
		calculate_struct_memory_layout({members, 2}, ret.size, ret.alignment);
		StructurePropertyDesc props[2] = {
			{"first", first, members[0].offset},
			{"second", second, members[1].offset},
		};
		ret.base_type = nullptr;
		ret.properties.assign_n(props, 2);
		ret.ctor = nullptr;
		ret.dtor = nullptr;
		ret.copy_ctor = nullptr;
		ret.move_ctor = nullptr;
		ret.copy_assign = nullptr;
		ret.move_assign = nullptr;
		ret.trivially_relocatable = is_type_trivially_relocatable(first) && is_type_trivially_relocatable(second);
		return ret;
	}
	static R<Variant> serialize_pair(typeinfo_t type, const void* inst)
	{
		Variant ret(VariantType::array);
		lutry
		{
			auto props = get_struct_properties(type);
			luassert(props.size() >= 2);
			lulet(data, serialize(props[0].type, (const void*)((usize)inst + props[0].offset)));
			ret.push_back(move(data));
			luset(data, serialize(props[1].type, (const void*)((usize)inst + props[1].offset)));
			ret.push_back(move(data));
		}
		lucatchret;
		return ret;
	}
	static RV deserialize_pair(typeinfo_t type, void* inst, const Variant& data)
	{
		auto props = get_struct_properties(type);
		luassert(props.size() >= 2);
		lutry
		{
			luexp(deserialize(props[0].type, (void*)((usize)inst + props[0].offset), data[0]));
			luexp(deserialize(props[1].type, (void*)((usize)inst + props[1].offset), data[1]));
		}
		lucatchret;
		return ok;
	}
	static GenericStructureInstantiateInfo tuple_instantiate(typeinfo_t base_type, Span<const typeinfo_t> generic_arguments)
	{
		Vector<MemoryLayoutMember> members;
		members.reserve(generic_arguments.size());
		for (usize i = 0; i < generic_arguments.size(); ++i)
		{
			members.push_back(MemoryLayoutMember(get_type_size(generic_arguments[i]), get_type_alignment(generic_arguments[i])));
		}
		GenericStructureInstantiateInfo ret;
		calculate_struct_memory_layout({members.data(), members.size()}, ret.size, ret.alignment);
		StructurePropertyDesc prop;
		ret.properties = Array<StructurePropertyDesc>(members.size());
		for (u32 i = 0; i < members.size(); ++i)
		{
			c8 name_buf[8];
			snprintf(name_buf, 8, "%u", i);
			prop.name = name_buf;
			prop.type = generic_arguments[i];
			prop.offset = members[i].offset;
			ret.properties[i] = prop;
		}
		ret.base_type = nullptr;
		ret.ctor = nullptr;
		ret.dtor = nullptr;
		ret.copy_ctor = nullptr;
		ret.move_ctor = nullptr;
		ret.copy_assign = nullptr;
		ret.move_assign = nullptr;
		ret.trivially_relocatable = true;
		for (usize i = 0; i < generic_arguments.size(); ++i)
		{
			ret.trivially_relocatable &= is_type_trivially_relocatable(generic_arguments[i]);
		}
		return ret;
	}
	static R<Variant> serialize_tuple(typeinfo_t type, const void* inst)
	{
		Variant ret(VariantType::array);
		lutry
		{
			auto properties = get_struct_properties(type);
			for (auto& prop : properties)
			{
				lulet(data, serialize(prop.type, (const void*)((usize)inst + prop.offset)));
				ret.push_back(move(data));
			}
		}
		lucatchret;
		return ret;
	}
	static RV deserialize_tuple(typeinfo_t type, void* inst, const Variant& data)
	{
		auto properties = get_struct_properties(type);
		lutry
		{
			for (usize i = 0; i < properties.size(); ++i)
			{
				auto& prop = properties[i];
				luexp(deserialize(prop.type, (void*)((usize)inst + prop.offset), data[i]));
			}
		}
		lucatchret;
		return ok;
	}
	inline usize robinhood_insert(usize h, typeinfo_t value_type, usize value_size, usize value_alignment, void* src_buf, void* value_buf, RobinHoodHashing::ControlBlock* cb_buf, usize buffer_size)
	{
		luassert(h != RobinHoodHashing::EMPTY_SLOT && !RobinHoodHashing::is_tombstone(h));
		// Extract current data.
		usize pos = h % buffer_size;
		usize dist = 0;
		usize ret_pos = USIZE_MAX;
		void* temp_v = alloca(value_size + value_alignment);
		temp_v = (void*)align_upper((usize)temp_v, value_alignment);
		while (true)
		{
			if (cb_buf[pos].m_hash == RobinHoodHashing::EMPTY_SLOT)
			{
				cb_buf[pos].m_hash = h;
				void* dst = (void*)((usize)value_buf + pos * value_size);
				relocate_type(value_type, dst, src_buf);
				if (ret_pos == USIZE_MAX) ret_pos = pos;
				break;
			}
			usize existing_dist = RobinHoodHashing::probe_distance(cb_buf[pos].m_hash, pos, buffer_size);
			if ((existing_dist <= dist) && RobinHoodHashing::is_tombstone(cb_buf[pos].m_hash))
			{
				cb_buf[pos].m_hash = h;
				void* dst = (void*)((usize)value_buf + pos * value_size);
				relocate_type(value_type, dst, src_buf);
				if (ret_pos == USIZE_MAX) ret_pos = pos;
				break;
			}
			if (existing_dist < dist)
			{
				usize temp_h = cb_buf[pos].m_hash;
				cb_buf[pos].m_hash = h;
				h = temp_h;
				void* dst = (void*)((usize)value_buf + pos * value_size);
				relocate_type(value_type, temp_v, dst);
				relocate_type(value_type, dst, src_buf);
				relocate_type(value_type, src_buf, temp_v);
				dist = existing_dist;
				if (ret_pos == USIZE_MAX) ret_pos = pos;
			}
			++pos;
			++dist;
			if (pos == buffer_size)
				pos = 0;
		}
		return ret_pos;
	}
	struct HashTableData
	{
		void* m_value_buffer;
		RobinHoodHashing::ControlBlock* m_cb_buffer;
		usize m_buffer_size;
		usize m_size;
		f32 m_max_load_factor;

		void clear_and_free_table(typeinfo_t value_type)
		{
			for (usize i = 0; i < m_buffer_size; ++i)
			{
				usize h = m_cb_buffer[i].m_hash;
				if (h == RobinHoodHashing::EMPTY_SLOT || RobinHoodHashing::is_tombstone(h)) continue;
				destruct_type(value_type, (void*)((usize)m_value_buffer + i * get_type_size(value_type)));
			}
			if (m_value_buffer)
			{
				memfree(m_value_buffer);
				memfree(m_cb_buffer);
				m_value_buffer = nullptr;
			}
			m_buffer_size = 0;
			m_size = 0;
		}
		f32 load_factor()
		{
			if (!m_buffer_size)
			{
				return 0.0f;
			}
			return (f32)m_size / (f32)m_buffer_size;
		}
		void rehash(typeinfo_t value_type, usize new_buffer_size)
		{
			new_buffer_size = max(max(new_buffer_size, (usize)(ceilf((f32)m_size / m_max_load_factor))), RobinHoodHashing::INITIAL_BUFFER_SIZE);
			if (new_buffer_size == m_buffer_size)
			{
				return;
			}
			usize value_size = get_type_size(value_type);
			usize value_alignment = get_type_alignment(value_type);
			void* value_buf = memalloc(new_buffer_size * value_size, value_alignment);
			RobinHoodHashing::ControlBlock* cb_buf = (RobinHoodHashing::ControlBlock*)memalloc(new_buffer_size * sizeof(RobinHoodHashing::ControlBlock));
			memzero(cb_buf, new_buffer_size * sizeof(RobinHoodHashing::ControlBlock));
			for (usize i = 0; i < m_buffer_size; ++i)
			{
				usize h = m_cb_buffer[i].m_hash;
				if (h == RobinHoodHashing::EMPTY_SLOT || RobinHoodHashing::is_tombstone(h)) continue;
				void* src = (void*)((usize)m_value_buffer + i * value_size);
				robinhood_insert(h, value_type, value_size, value_alignment, src, value_buf, cb_buf, new_buffer_size);
			}
			if (m_value_buffer)
			{
				memfree(m_value_buffer);
				memfree(m_cb_buffer);
				m_value_buffer = nullptr;
			}
			m_value_buffer = value_buf;
			m_cb_buffer = cb_buf;
			m_buffer_size = new_buffer_size;
		}
		usize capacity() const
		{
			return (usize)floorf(m_max_load_factor * m_buffer_size);
		}
		void increment_reserve(typeinfo_t value_type, usize new_cap)
		{
			usize current_capacity = capacity();
			if (new_cap > current_capacity)
			{
				new_cap = max(new_cap, current_capacity * 2);
				rehash(value_type, (usize)ceilf((f32)new_cap / m_max_load_factor));
			}
		}
		R<Variant> do_serialize(typeinfo_t value_type) const
		{
			usize value_size = get_type_size(value_type);
			Variant ret(VariantType::array);
			lutry
			{
				for (usize i = 0; i < m_buffer_size; ++i)
				{
					usize h = m_cb_buffer[i].m_hash;
					if (h != RobinHoodHashing::EMPTY_SLOT && !RobinHoodHashing::is_tombstone(h))
					{
						const void* v = (const void*)((usize)m_value_buffer + value_size * i);
						lulet(var, serialize(value_type, v));
						ret.push_back(move(var));
					}
				}
			}
			lucatchret;
			return ret;
		}
	};
	static typeinfo_t g_guid_type;
	static typeinfo_t g_version_type;
	static typeinfo_t g_string_type;
	static typeinfo_t g_name_type;
	static typeinfo_t g_vector_type;
	static typeinfo_t g_path_type;
	static typeinfo_t g_pair_type;
	static typeinfo_t g_tuple_type;
	static typeinfo_t g_hash_map_type;
	static typeinfo_t g_hash_set_type;
	static typeinfo_t g_float2_type;
	static typeinfo_t g_float3_type;
	static typeinfo_t g_float4_type;
	static typeinfo_t g_vec2u_type;
	static typeinfo_t g_vec3u_type;
	static typeinfo_t g_vec4u_type;
	static typeinfo_t g_float3x3_type;
	static typeinfo_t g_float4x4_type;
	static typeinfo_t g_quaternion_type;
	static typeinfo_t g_blob_type;

	inline typeinfo_t make_hashmap_value_type(typeinfo_t key_type, typeinfo_t value_type)
	{
		return get_generic_instanced_type(g_pair_type, { key_type, value_type });
	}
	static void hashtable_ctor(typeinfo_t type, void* inst)
	{
		HashTableData* d = (HashTableData*)inst;
		d->m_value_buffer = nullptr;
		d->m_cb_buffer = nullptr;
		d->m_buffer_size = 0;
		d->m_size = 0;
		d->m_max_load_factor = RobinHoodHashing::INITIAL_LOAD_FACTOR;
	}
	static void hashmap_dtor(typeinfo_t type, void* inst)
	{
		HashTableData* d = (HashTableData*)inst;
		auto generic_arguments = get_struct_generic_arguments(type);
		typeinfo_t value_type = make_hashmap_value_type(generic_arguments[0], generic_arguments[1]);
		d->clear_and_free_table(value_type);
	}
	static void hashset_dtor(typeinfo_t type, void* inst)
	{
		HashTableData* d = (HashTableData*)inst;
		typeinfo_t value_type = get_struct_generic_arguments(type)[0];
		d->clear_and_free_table(value_type);
	}
	inline void hashtable_copy_ctor(typeinfo_t value_type, HashTableData* dest, HashTableData* src)
	{
		dest->m_value_buffer = nullptr;
		dest->m_cb_buffer = nullptr;
		dest->m_buffer_size = 0;
		dest->m_size = 0;
		dest->m_max_load_factor = RobinHoodHashing::INITIAL_LOAD_FACTOR;
		usize value_size = get_type_size(value_type);
		dest->m_max_load_factor = src->m_max_load_factor;
		if (dest->load_factor() > dest->m_max_load_factor)
		{
			dest->rehash(value_type, 0);
		}
		if (src->m_size)
		{
			dest->m_value_buffer = memalloc(src->m_buffer_size * value_size, get_type_alignment(value_type));
			dest->m_cb_buffer = (RobinHoodHashing::ControlBlock*)memalloc(src->m_buffer_size * sizeof(RobinHoodHashing::ControlBlock));
			memzero(dest->m_cb_buffer, src->m_buffer_size * sizeof(RobinHoodHashing::ControlBlock));
			dest->m_buffer_size = src->m_buffer_size;
			for (usize i = 0; i < src->m_buffer_size; ++i)
			{
				usize h = src->m_cb_buffer[i].m_hash;
				dest->m_cb_buffer[i].m_hash = h;
				if (h != RobinHoodHashing::EMPTY_SLOT && !RobinHoodHashing::is_tombstone(h))
				{
					void* dest_buf = (void*)((usize)dest->m_value_buffer + i * value_size);
					void* src_buf = (void*)((usize)src->m_value_buffer + i * value_size);
					copy_construct_type(value_type, dest_buf, src_buf);
				}
			}
			dest->m_size = src->m_size;
		}
	}
	static void hashmap_copy_ctor(typeinfo_t type, void* dest, void* src)
	{
		auto generic_arguments = get_struct_generic_arguments(type);
		typeinfo_t value_type = make_hashmap_value_type(generic_arguments[0], generic_arguments[1]);
		hashtable_copy_ctor(value_type, (HashTableData*)dest, (HashTableData*)src);
	}
	static void hashset_copy_ctor(typeinfo_t type, void* dest, void* src)
	{
		typeinfo_t value_type = get_struct_generic_arguments(type)[0];
		hashtable_copy_ctor(value_type, (HashTableData*)dest, (HashTableData*)src);
	}
	static void hashtable_move_ctor(typeinfo_t type, void* dest, void* src)
	{
		HashTableData* destd = (HashTableData*)dest;
		HashTableData* srcd = (HashTableData*)src;
		memcpy(destd, srcd, sizeof(HashTableData));
		srcd->m_value_buffer = nullptr;
		srcd->m_buffer_size = 0;
		srcd->m_size = 0;
	}
	static void hashmap_copy_assign(typeinfo_t type, void* dest, void* src)
	{
		hashmap_dtor(type, dest);
		hashmap_copy_ctor(type, dest, src);
	}
	static void hashset_copy_assign(typeinfo_t type, void* dest, void* src)
	{
		hashset_dtor(type, dest);
		hashset_copy_ctor(type, dest, src);
	}
	static void hashmap_move_assign(typeinfo_t type, void* dest, void* src)
	{
		hashmap_dtor(type, dest);
		hashtable_move_ctor(type, dest, src);
	}
	static void hashset_move_assign(typeinfo_t type, void* dest, void* src)
	{
		hashset_dtor(type, dest);
		hashtable_move_ctor(type, dest, src);
	}
	static R<Variant> hashmap_serialize(typeinfo_t type, const void* inst)
	{
		const HashTableData* d = (const HashTableData*)inst;
		auto generic_arguments = get_struct_generic_arguments(type);
		typeinfo_t value_type = make_hashmap_value_type(generic_arguments[0], generic_arguments[1]);
		return d->do_serialize(value_type);
	}
	inline usize alter_hash(usize h)
	{
		if (h == RobinHoodHashing::EMPTY_SLOT) ++h;
		return h & ~RobinHoodHashing::TOMBSTONE_BIT;
	}
	static RV hashmap_deserialize(typeinfo_t type, void* inst, const Variant& data)
	{
		lutry
		{
			HashTableData* d = (HashTableData*)inst;
			auto generic_arguments = get_struct_generic_arguments(type);
			typeinfo_t key_type = generic_arguments[0];
			typeinfo_t value_type = make_hashmap_value_type(key_type, generic_arguments[1]);
			usize value_size = get_type_size(value_type);
			usize value_alignment = get_type_alignment(value_type);
			void* value_buffer = alloca(value_size + value_alignment);
			value_buffer = (void*)align_upper((usize)value_buffer, value_alignment);
			for (auto& v : data.values())
			{
				construct_type(value_type, value_buffer);
				luexp(deserialize(value_type, value_buffer, v));
				// Extract key type.
				usize h = alter_hash(hash_type(key_type, value_buffer));
				d->increment_reserve(value_type, d->m_size + 1);
				robinhood_insert(h, value_type, value_size, value_alignment, value_buffer, d->m_value_buffer, d->m_cb_buffer, d->m_buffer_size);
				++d->m_size;
			}
		}
		lucatchret;
		return ok;
	}
	static R<Variant> hashset_serialize(typeinfo_t type, const void* inst)
	{
		const HashTableData* d = (const HashTableData*)inst;
		typeinfo_t value_type = get_struct_generic_arguments(type)[0];
		return d->do_serialize(value_type);
	}
	static RV hashset_deserialize(typeinfo_t type, void* inst, const Variant& data)
	{
		lutry
		{
			HashTableData* d = (HashTableData*)inst;
			typeinfo_t value_type = get_struct_generic_arguments(type)[0];
			usize value_size = get_type_size(value_type);
			usize value_alignment = get_type_alignment(value_type);
			void* value_buffer = alloca(value_size + value_alignment);
			value_buffer = (void*)align_upper((usize)value_buffer, value_alignment);
			for (auto& v : data.values())
			{
				construct_type(value_type, value_buffer);
				luexp(deserialize(value_type, value_buffer, v));
				// Extract key type.
				usize h = alter_hash(hash_type(value_type, value_buffer));
				d->increment_reserve(value_type, d->m_size + 1);
				robinhood_insert(h, value_type, value_size, value_alignment, value_buffer, d->m_value_buffer, d->m_cb_buffer, d->m_buffer_size);
				++d->m_size;
			}
		}
		lucatchret;
		return ok;
	}
	static GenericStructureInstantiateInfo hashmap_instantiate(typeinfo_t base_type, Span<const typeinfo_t> generic_arguments)
	{
		GenericStructureInstantiateInfo ret;
		ret.size = sizeof(HashTableData);
		ret.alignment = alignof(HashTableData);
		ret.base_type = nullptr;
		ret.ctor = hashtable_ctor;
		ret.dtor = hashmap_dtor;
		ret.copy_ctor = hashmap_copy_ctor;
		ret.move_ctor = hashtable_move_ctor;
		ret.copy_assign = hashmap_copy_assign;
		ret.move_assign = hashmap_move_assign;
		ret.trivially_relocatable = true;
		return ret;
	}
	static GenericStructureInstantiateInfo hashset_instantiate(typeinfo_t base_type, Span<const typeinfo_t> generic_arguments)
	{
		GenericStructureInstantiateInfo ret;
		ret.size = sizeof(HashTableData);
		ret.alignment = alignof(HashTableData);
		ret.base_type = nullptr;
		ret.ctor = hashtable_ctor;
		ret.dtor = hashset_dtor;
		ret.copy_ctor = hashset_copy_ctor;
		ret.move_ctor = hashtable_move_ctor;
		ret.copy_assign = hashset_copy_assign;
		ret.move_assign = hashset_move_assign;
		ret.trivially_relocatable = true;
		return ret;
	}
	static R<Variant> quaternion_serialize(typeinfo_t type, const void* inst)
	{
		const Quaternion* q = (const Quaternion*)inst;
		Variant r(VariantType::array);
		r.push_back(q->x);
		r.push_back(q->y);
		r.push_back(q->z);
		r.push_back(q->w);
		return r;
	}
	void add_builtin_typeinfo()
	{
		// Primitive types.
		{
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_u8;
			serial.deserialize_func = deserialize_u8;
			set_serializable(u8_type(), &serial);
			set_equatable(u8_type(), default_equal_to<u8>);
			set_hashable(u8_type(), default_hash<u8>);
			serial.serialize_func = serialize_i8;
			serial.deserialize_func = deserialize_i8;
			set_serializable(i8_type(), &serial);
			set_equatable(i8_type(), default_equal_to<i8>);
			set_hashable(i8_type(), default_hash<i8>);
			serial.serialize_func = serialize_u16;
			serial.deserialize_func = deserialize_u16;
			set_serializable(u16_type(), &serial);
			set_equatable(u16_type(), default_equal_to<u16>);
			set_hashable(u16_type(), default_hash<u16>);
			serial.serialize_func = serialize_i16;
			serial.deserialize_func = deserialize_i16;
			set_serializable(i16_type(), &serial);
			set_equatable(i16_type(), default_equal_to<i16>);
			set_hashable(i16_type(), default_hash<i16>);
			serial.serialize_func = serialize_u32;
			serial.deserialize_func = deserialize_u32;
			set_serializable(u32_type(), &serial);
			set_equatable(u32_type(), default_equal_to<u32>);
			set_hashable(u32_type(), default_hash<u32>);
			serial.serialize_func = serialize_i32;
			serial.deserialize_func = deserialize_i32;
			set_serializable(i32_type(), &serial);
			set_equatable(i32_type(), default_equal_to<i32>);
			set_hashable(i32_type(), default_hash<i32>);
			serial.serialize_func = serialize_u64;
			serial.deserialize_func = deserialize_u64;
			set_serializable(u64_type(), &serial);
			set_equatable(u64_type(), default_equal_to<u64>);
			set_hashable(u64_type(), default_hash<u64>);
			serial.serialize_func = serialize_i64;
			serial.deserialize_func = deserialize_i64;
			set_serializable(i64_type(), &serial);
			set_equatable(i64_type(), default_equal_to<i64>);
			set_hashable(i64_type(), default_hash<i64>);
			serial.serialize_func = serialize_usize;
			serial.deserialize_func = deserialize_usize;
			set_serializable(usize_type(), &serial);
			set_equatable(usize_type(), default_equal_to<usize>);
			set_hashable(usize_type(), default_hash<usize>);
			serial.serialize_func = serialize_isize;
			serial.deserialize_func = deserialize_isize;
			set_serializable(isize_type(), &serial);
			set_equatable(isize_type(), default_equal_to<isize>);
			set_hashable(isize_type(), default_hash<isize>);
			serial.serialize_func = serialize_f32;
			serial.deserialize_func = deserialize_f32;
			set_serializable(f32_type(), &serial);
			set_equatable(f32_type(), default_equal_to<f32>);
			set_hashable(f32_type(), default_hash<f32>);
			serial.serialize_func = serialize_f64;
			serial.deserialize_func = deserialize_f64;
			set_serializable(f64_type(), &serial);
			set_equatable(f64_type(), default_equal_to<f64>);
			set_hashable(f64_type(), default_hash<f64>);
			serial.serialize_func = serialize_c8;
			serial.deserialize_func = deserialize_c8;
			set_serializable(c8_type(), &serial);
			set_equatable(c8_type(), default_equal_to<c8>);
			set_hashable(c8_type(), default_hash<c8>);
			serial.serialize_func = serialize_c16;
			serial.deserialize_func = deserialize_c16;
			set_serializable(c16_type(), &serial);
			set_equatable(c16_type(), default_equal_to<c16>);
			set_hashable(c16_type(), default_hash<c16>);
			serial.serialize_func = serialize_c32;
			serial.deserialize_func = deserialize_c32;
			set_serializable(c32_type(), &serial);
			set_equatable(c32_type(), default_equal_to<c32>);
			set_hashable(c32_type(), default_hash<c32>);
			serial.serialize_func = serialize_bool;
			serial.deserialize_func = deserialize_bool;
			set_serializable(boolean_type(), &serial);
			set_equatable(boolean_type(), default_equal_to<bool>);
			set_hashable(boolean_type(), default_hash<bool>);
			serial.serialize_func = serialize_usize;
			serial.deserialize_func = deserialize_usize;
		}
		// Guid
		{
			StructureTypeDesc desc;
			desc.guid = Guid("{7C0FD89E-174E-46F0-A072-C6C2CCF452F2}");
			desc.name = "Guid";
			desc.alias = "";
			desc.size = sizeof(Guid);
			desc.alignment = alignof(Guid);
			desc.base_type = nullptr;
			desc.ctor = nullptr;
			desc.dtor = nullptr;
			desc.copy_ctor = nullptr;
			desc.move_ctor = nullptr;
			desc.copy_assign = nullptr;
			desc.move_assign = nullptr;
			desc.trivially_relocatable = true;
			StructurePropertyDesc props[2] = {

			};
			props[0].name = "high";
			props[0].type = u64_type();
			props[0].offset = offsetof(Guid, high);
			props[1].name = "low";
			props[1].type = u64_type();
			props[1].offset = offsetof(Guid, low);
			desc.properties = {props, 2};
			typeinfo_t type = register_struct_type(desc);
			g_guid_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_guid;
			serial.deserialize_func = deserialize_guid;
			set_serializable(type, &serial);
			set_equatable(type, default_equal_to<Guid>);
			set_hashable(type, default_hash<Guid>);
		}
		// Version
		{
			StructureTypeDesc desc;
			desc.guid = Guid("{FA46B660-EEDC-4D46-B31A-779C6668ED19}");
			desc.name = "Version";
			desc.alias = "";
			desc.size = sizeof(Version);
			desc.alignment = alignof(Version);
			desc.base_type = nullptr;
			desc.ctor = nullptr;
			desc.dtor = nullptr;
			desc.copy_ctor = nullptr;
			desc.move_ctor = nullptr;
			desc.copy_assign = nullptr;
			desc.move_assign = nullptr;
			desc.trivially_relocatable = true;
			StructurePropertyDesc props[3] = {
				{"major", u32_type(), offsetof(Version, major)},
				{"minor", u32_type(), offsetof(Version, minor)},
				{"patch", u32_type(), offsetof(Version, patch)}
			};
			desc.properties = {props, 3};
			typeinfo_t type = register_struct_type(desc);
			g_version_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_version;
			serial.deserialize_func = deserialize_version;
			set_serializable(type, &serial);
			set_equatable(type, default_equal_to<Version>);
		}
		// String
		{
			StructureTypeDesc desc;
			desc.guid = Guid("{BA5C6C94-6651-4DAC-A018-B2B117CEB93B}");
			desc.name = "String";
			desc.alias = "";
			desc.size = sizeof(String);
			desc.alignment = alignof(String);
			desc.base_type = nullptr;
			desc.ctor = nullptr;
			desc.dtor = default_dtor<String>;
			desc.copy_ctor = default_copy_ctor<String>;
			desc.move_ctor = default_move_ctor<String>;
			desc.copy_assign = default_copy_assign<String>;
			desc.move_assign = default_move_assign<String>;
			desc.trivially_relocatable = true;
			typeinfo_t type = register_struct_type(desc);
			g_string_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_string;
			serial.deserialize_func = deserialize_string;
			set_serializable(type, &serial);
		}
		// Name
		{
			StructureTypeDesc desc;
			desc.guid = Guid("{E5EEA2C6-2D51-4658-9B3F-C141DDE983D8}");
			desc.name = "Name";
			desc.alias = "";
			desc.size = sizeof(Name);
			desc.alignment = alignof(Name);
			desc.base_type = nullptr;
			desc.ctor = nullptr;
			desc.dtor = default_dtor<Name>;
			desc.copy_ctor = default_copy_ctor<Name>;
			desc.move_ctor = default_move_ctor<Name>;
			desc.copy_assign = default_copy_assign<Name>;
			desc.move_assign = default_move_assign<Name>;
			desc.trivially_relocatable = true;
			typeinfo_t type = register_struct_type(desc);
			g_name_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_name;
			serial.deserialize_func = deserialize_name;
			set_serializable(type, &serial);
			set_equatable(type, default_equal_to<Name>);
			set_hashable(type, default_hash<Name>);
		}
		// Vector
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{F7ED58B5-5473-4A12-B69D-3A122FA1E60C}");
			desc.name = "Vector";
			desc.alias = "";
			Name generic_parameter_names[1] = {"ElementType"};
			desc.generic_parameter_names = {generic_parameter_names, 1};
			desc.variable_generic_parameters = false;
			desc.instantiate = vector_instantiate;
			typeinfo_t type = register_generic_struct_type(desc);
			g_vector_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_vector;
			serial.deserialize_func = deserialize_vector;
			set_serializable(type, &serial);
		}
		// Path
		{
			StructureTypeDesc desc;
			desc.guid = Guid("{BAD6FC9B-C426-466E-866B-2F4CA0D01C69}");
			desc.name = "Path";
			desc.alias = "";
			desc.size = sizeof(Path);
			desc.alignment = alignof(Path);
			desc.base_type = nullptr;
			desc.ctor = nullptr;
			desc.dtor = default_dtor<Path>;
			desc.copy_ctor = default_copy_ctor<Path>;
			desc.move_ctor = default_move_ctor<Path>;
			desc.copy_assign = default_copy_assign<Path>;
			desc.move_assign = default_move_assign<Path>;
			desc.trivially_relocatable = true;
			typeinfo_t type = register_struct_type(desc);
			g_path_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_path;
			serial.deserialize_func = deserialize_path;
			set_serializable(type, &serial);
			set_equatable(type, default_equal_to<Path>);
			set_hashable(type, default_hash<Path>);
		}
		// Pair
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{E2C85F8B-56DB-45BA-BBBA-AB36E09ED795}");
			desc.name = "Pair";
			desc.alias = "";
			Name generic_parameter_names[2] = {"FirstElementType", "SecondElementType"};
			desc.generic_parameter_names = {generic_parameter_names, 2};
			desc.variable_generic_parameters = false;
			desc.instantiate = pair_instantiate;
			typeinfo_t type = register_generic_struct_type(desc);
			g_pair_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_pair;
			serial.deserialize_func = deserialize_pair;
			set_serializable(type, &serial);
		}
		// Tuple
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{F577F1CC-1890-4A46-895B-DAF2C4678A04}");
			desc.name = "Tuple";
			desc.alias = "";
			Name generic_parameter_names[1] = {"FirstElementType"};
			desc.generic_parameter_names = {generic_parameter_names, 1}; // The tuple shall have at least one generic argument.
			desc.variable_generic_parameters = true;
			desc.instantiate = tuple_instantiate;
			typeinfo_t type = register_generic_struct_type(desc);
			g_tuple_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = serialize_tuple;
			serial.deserialize_func = deserialize_tuple;
			set_serializable(type, &serial);
		}
		// HashMap
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{40563229-68C4-48B3-ACAF-C7659B35DE88}");
			desc.name = "HashMap";
			desc.alias = "";
			Name generic_parameter_names[2] = {"KeyType", "ValueType"};
			desc.generic_parameter_names = {generic_parameter_names, 2};
			desc.variable_generic_parameters = false;
			desc.instantiate = hashmap_instantiate;
			typeinfo_t type = register_generic_struct_type(desc);
			g_hash_map_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = hashmap_serialize;
			serial.deserialize_func = hashmap_deserialize;
			set_serializable(type, &serial);
		}
		// HashSet
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{64356A48-BB74-4C7B-A43E-9D60E45B33E6}");
			desc.name = "HashSet";
			desc.alias = "";
			Name generic_parameter_names[1] = {"ElementType"};
			desc.generic_parameter_names = {generic_parameter_names, 1};
			desc.variable_generic_parameters = false;
			desc.instantiate = hashset_instantiate;
			typeinfo_t type = register_generic_struct_type(desc);
			g_hash_set_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = hashset_serialize;
			serial.deserialize_func = hashset_deserialize;
			set_serializable(type, &serial);
		}
		// Float2
		{
			g_float2_type = register_struct_type<Float2>({
				luproperty(Float2, f32, x),
				luproperty(Float2, f32, y)
				});
			SerializableTypeDesc desc;
			desc.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				auto v = (const Float2*)inst;
				Variant r(VariantType::array);
				r.push_back(v->x);
				r.push_back(v->y);
				return r;
			};
			desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data)
			{
				auto v = (Float2*)inst;
				v->x = (f32)data[0].fnum();
				v->y = (f32)data[1].fnum();
				return ok;
			};
			set_serializable(g_float2_type, &desc);
		}
		// Float3
		{
			g_float3_type = register_struct_type<Float3>({
				luproperty(Float3, f32, x),
				luproperty(Float3, f32, y),
				luproperty(Float3, f32, z)
				});
			SerializableTypeDesc desc;
			desc.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				auto v = (const Float3*)inst;
				Variant r(VariantType::array);
				r.push_back(v->x);
				r.push_back(v->y);
				r.push_back(v->z);
				return r;
			};
			desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data)
			{
				auto v = (Float3*)inst;
				v->x = (f32)data[0].fnum();
				v->y = (f32)data[1].fnum();
				v->z = (f32)data[2].fnum();
				return ok;
			};
			set_serializable(g_float3_type, &desc);
		}
		// Float4
		{
			g_float4_type = register_struct_type<Float4>({
				luproperty(Float4, f32, x),
				luproperty(Float4, f32, y),
				luproperty(Float4, f32, z),
				luproperty(Float4, f32, w)
				});
			SerializableTypeDesc desc;
			desc.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				auto v = (const Float4*)inst;
				Variant r(VariantType::array);
				r.push_back(v->x);
				r.push_back(v->y);
				r.push_back(v->z);
				r.push_back(v->w);
				return r;
			};
			desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data)
			{
				auto v = (Float4*)inst;
				v->x = (f32)data[0].fnum();
				v->y = (f32)data[1].fnum();
				v->z = (f32)data[2].fnum();
				v->w = (f32)data[3].fnum();
				return ok;
			};
			set_serializable(g_float4_type);
		}
		// Vec2U
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{5B81F926-E591-4DDA-9D04-F9603D9121EF}");
			desc.name = "Vec2U";
			desc.alias = "";
			Name generic_parameter_names[1] = { "ElementType" };
			desc.generic_parameter_names = {generic_parameter_names, 1};
			desc.variable_generic_parameters = false;
			desc.instantiate = [](typeinfo_t generic_type, Span<const typeinfo_t> generic_arguments)
			{
				GenericStructureInstantiateInfo info;
				lucheck(!generic_arguments.empty());
				typeinfo_t element_type = generic_arguments[0];
				usize element_size = get_type_size(element_type);
				info.size = element_size * 2;
				info.alignment = get_type_alignment(element_type);
				info.properties = {
					StructurePropertyDesc("x", element_type, 0),
					StructurePropertyDesc("y", element_type, element_size)
				};
				return info;
			};
			typeinfo_t type = register_generic_struct_type(desc);
			g_vec2u_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				Variant r(VariantType::array);
				lutry
				{
					auto properties = get_struct_properties(type);
					luassert(properties.size() >= 2);
					lulet(val, serialize(properties[0].type, (const void*)((usize)inst + properties[0].offset)));
					r.push_back(move(val));
					luset(val, serialize(properties[1].type, (const void*)((usize)inst + properties[1].offset)))
					r.push_back(move(val));
					return r;
				}
				lucatchret;
				return r;
			};
			serial.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data) -> RV
			{
				lutry
				{
					auto properties = get_struct_properties(type);
					luassert(properties.size() >= 2);
					luexp(deserialize(properties[0].type, (void*)((usize)inst + properties[0].offset), data[0]));
					luexp(deserialize(properties[1].type, (void*)((usize)inst + properties[1].offset), data[1]));
				}
				lucatchret;
				return ok;
			};
			set_serializable(type, &serial);
		}
		// Vec3U
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{045C93AB-4FB5-4010-BE09-B595BEC58CC5}");
			desc.name = "Vec3U";
			desc.alias = "";
			Name generic_parameter_names[1] = { "ElementType" };
			desc.generic_parameter_names = {generic_parameter_names, 1};
			desc.variable_generic_parameters = false;
			desc.instantiate = [](typeinfo_t generic_type, Span<const typeinfo_t> generic_arguments)
			{
				GenericStructureInstantiateInfo info;
				lucheck(!generic_arguments.empty());
				typeinfo_t element_type = generic_arguments[0];
				usize element_size = get_type_size(element_type);
				info.size = element_size * 3;
				info.alignment = get_type_alignment(element_type);
				info.properties = {
					StructurePropertyDesc("x", element_type, 0),
					StructurePropertyDesc("y", element_type, element_size),
					StructurePropertyDesc("z", element_type, element_size * 2)
				};
				return info;
			};
			typeinfo_t type = register_generic_struct_type(desc);
			g_vec3u_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				Variant r(VariantType::array);
				lutry
				{
					auto properties = get_struct_properties(type);
					luassert(properties.size() >= 3);
					lulet(val, serialize(properties[0].type, (const void*)((usize)inst + properties[0].offset)));
					r.push_back(move(val));
					luset(val, serialize(properties[1].type, (const void*)((usize)inst + properties[1].offset)))
					r.push_back(move(val));
					luset(val, serialize(properties[2].type, (const void*)((usize)inst + properties[2].offset)))
					r.push_back(move(val));
					return r;
				}
				lucatchret;
				return r;
			};
			serial.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data) -> RV
			{
				lutry
				{
					auto properties = get_struct_properties(type);
					luassert(properties.size() >= 3);
					luexp(deserialize(properties[0].type, (void*)((usize)inst + properties[0].offset), data[0]));
					luexp(deserialize(properties[1].type, (void*)((usize)inst + properties[1].offset), data[1]));
					luexp(deserialize(properties[2].type, (void*)((usize)inst + properties[2].offset), data[2]));
				}
				lucatchret;
				return ok;
			};
			set_serializable(type, &serial);
		}
		// Vec4U
		{
			GenericStructureTypeDesc desc;
			desc.guid = Guid("{FFC6016B-2156-4958-BBBC-E08A3F17E51C}");
			desc.name = "Vec4U";
			desc.alias = "";
			Name generic_parameter_names[1] = { "ElementType" };
			desc.generic_parameter_names = { generic_parameter_names, 1 };
			desc.variable_generic_parameters = false;
			desc.instantiate = [](typeinfo_t generic_type, Span<const typeinfo_t> generic_arguments)
			{
				GenericStructureInstantiateInfo info;
				lucheck(!generic_arguments.empty());
				typeinfo_t element_type = generic_arguments[0];
				usize element_size = get_type_size(element_type);
				info.size = element_size * 4;
				info.alignment = get_type_alignment(element_type);
				info.properties = {
					StructurePropertyDesc("x", element_type, 0),
					StructurePropertyDesc("y", element_type, element_size),
					StructurePropertyDesc("z", element_type, element_size * 2),
					StructurePropertyDesc("w", element_type, element_size * 3)
				};
				return info;
			};
			typeinfo_t type = register_generic_struct_type(desc);
			g_vec4u_type = type;
			SerializableTypeDesc serial;
			serial.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				Variant r(VariantType::array);
				lutry
				{
					auto properties = get_struct_properties(type);
					luassert(properties.size() >= 4);
					lulet(val, serialize(properties[0].type, (const void*)((usize)inst + properties[0].offset)));
					r.push_back(move(val));
					luset(val, serialize(properties[1].type, (const void*)((usize)inst + properties[1].offset)))
					r.push_back(move(val));
					luset(val, serialize(properties[2].type, (const void*)((usize)inst + properties[2].offset)))
					r.push_back(move(val));
					luset(val, serialize(properties[3].type, (const void*)((usize)inst + properties[3].offset)))
					r.push_back(move(val));
					return r;
				}
				lucatchret;
				return r;
			};
			serial.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data) -> RV
			{
				lutry
				{
					auto properties = get_struct_properties(type);
					luassert(properties.size() >= 4);
					luexp(deserialize(properties[0].type, (void*)((usize)inst + properties[0].offset), data[0]));
					luexp(deserialize(properties[1].type, (void*)((usize)inst + properties[1].offset), data[1]));
					luexp(deserialize(properties[2].type, (void*)((usize)inst + properties[2].offset), data[2]));
					luexp(deserialize(properties[3].type, (void*)((usize)inst + properties[3].offset), data[3]));
				}
				lucatchret;
				return ok;
			};
			set_serializable(type, &serial);
		}
		// Float3x3
		{
			g_float3x3_type = register_struct_type<Float3x3>({
				{"r0", typeof<Float3>(), offsetof(Float3x3, r[0])},
				{"r1", typeof<Float3>(), offsetof(Float3x3, r[1])},
				{"r2", typeof<Float3>(), offsetof(Float3x3, r[2])}
				});
			set_serializable(g_float3x3_type);
		}
		// Float4x4
		{
			g_float4x4_type = register_struct_type<Float4x4>({
				{"r0", typeof<Float4>(), offsetof(Float4x4, r[0])},
				{"r1", typeof<Float4>(), offsetof(Float4x4, r[1])},
				{"r2", typeof<Float4>(), offsetof(Float4x4, r[2])},
				{"r3", typeof<Float4>(), offsetof(Float4x4, r[3])}
				});
			set_serializable(g_float4x4_type);
		}
		// Quaternion
		{
			g_quaternion_type = register_struct_type<Quaternion>({
				luproperty(Quaternion, f32, x),
				luproperty(Quaternion, f32, y),
				luproperty(Quaternion, f32, z),
				luproperty(Quaternion, f32, w)
				});
			SerializableTypeDesc desc;
			desc.serialize_func = [](typeinfo_t type, const void* inst) -> R<Variant>
			{
				auto q = (const Quaternion*)inst;
				Variant r(VariantType::array);
				r.push_back(q->x);
				r.push_back(q->y);
				r.push_back(q->z);
				r.push_back(q->w);
				return r;
			};
			desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data)
			{
				auto q = (Quaternion*)inst;
				q->x = (f32)data[0].fnum();
				q->y = (f32)data[1].fnum();
				q->z = (f32)data[2].fnum();
				q->w = (f32)data[3].fnum();
				return ok;
			};
			set_serializable(g_quaternion_type, &desc);
		}
		// Blob
		{
			StructureTypeDesc t;
			t.guid = Guid("{AD93BE44-C33F-458E-972E-6B8FE9E34D97}");
			t.name = "Blob";
			t.alias = "";
			t.size = sizeof(Blob);
			t.alignment = alignof(Blob);
			t.ctor = is_trivially_constructible_v<Blob> ? nullptr : default_ctor<Blob>;
			t.dtor = is_trivially_destructible_v<Blob> ? nullptr : default_dtor<Blob>;
			t.copy_ctor = is_trivially_copy_constructible_v<Blob> ? nullptr : default_copy_ctor<Blob>;
			t.move_ctor = is_trivially_move_constructible_v<Blob> ? nullptr : default_move_ctor<Blob>;
			t.copy_assign = is_trivially_copy_assignable_v<Blob> ? nullptr : default_copy_assign<Blob>;
			t.move_assign = is_trivially_move_assignable_v<Blob> ? nullptr : default_move_assign<Blob>;
			t.trivially_relocatable = true;
			g_blob_type = register_struct_type(t);
			SerializableTypeDesc desc;
			desc.serialize_func = [](typeinfo_t type, const void* inst)->R<Variant>
			{
				auto v = (const Blob*)inst;
				return Variant(*v);
			};
			desc.deserialize_func = [](typeinfo_t type, void* inst, const Variant& data) -> RV
			{
				auto v = (Blob*)inst;
				if (data.type() == VariantType::blob)
				{
					*v = Blob(data.blob_data(), data.blob_size(), data.blob_alignment());
					return ok;
				}
				return BasicError::bad_data();
			};
			set_serializable(g_blob_type, &desc);
		}
	}

	LUNA_RUNTIME_API typeinfo_t guid_type() { return g_guid_type; }
	LUNA_RUNTIME_API typeinfo_t version_type() { return g_version_type; }
	LUNA_RUNTIME_API typeinfo_t string_type() { return g_string_type; }
	LUNA_RUNTIME_API typeinfo_t name_type() { return g_name_type; }
	LUNA_RUNTIME_API typeinfo_t vector_type() { return g_vector_type; }
	LUNA_RUNTIME_API typeinfo_t path_type() { return g_path_type; }
	LUNA_RUNTIME_API typeinfo_t pair_type() { return g_pair_type; }
	LUNA_RUNTIME_API typeinfo_t tuple_type() { return g_tuple_type; }
	LUNA_RUNTIME_API typeinfo_t hash_map_type() { return g_hash_map_type; }
	LUNA_RUNTIME_API typeinfo_t hash_set_type() { return g_hash_set_type; }
	LUNA_RUNTIME_API typeinfo_t float2_type() { return g_float2_type; }
	LUNA_RUNTIME_API typeinfo_t float3_type() { return g_float3_type; }
	LUNA_RUNTIME_API typeinfo_t float4_type() { return g_float4_type; }
	LUNA_RUNTIME_API typeinfo_t vec2u_type() { return g_vec2u_type; }
	LUNA_RUNTIME_API typeinfo_t vec3u_type() { return g_vec3u_type; }
	LUNA_RUNTIME_API typeinfo_t vec4u_type() { return g_vec4u_type; }
	LUNA_RUNTIME_API typeinfo_t float3x3_type() { return g_float3x3_type; }
	LUNA_RUNTIME_API typeinfo_t float4x4_type() { return g_float4x4_type; }
	LUNA_RUNTIME_API typeinfo_t quaternion_type() { return g_quaternion_type; }
	LUNA_RUNTIME_API typeinfo_t blob_type() { return g_blob_type; }
}
