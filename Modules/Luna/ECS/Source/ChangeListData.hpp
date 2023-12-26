/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ChangeListData.hpp
* @author JXMaster
* @date 2022/9/8
*/
#pragma once
#include "../ECS.hpp"
namespace Luna
{
	namespace ECS
	{
		enum class ChangeListOpType : u8
		{
			add_entity = 0,
			remove_entity,
			remove_all_entities,
			set_target_entity,
			add_component,
			add_component_if_not_exists,
			set_component, // currently not used.
			remove_component,
			remove_all_components,
			add_tag,
			remove_tag,
			remove_all_tags
		};
		struct ComponentBuffer
		{
			typeinfo_t m_type;
			void* m_data = nullptr;
			usize m_size = 0;
			usize m_capacity = 0;

			ComponentBuffer(typeinfo_t type) :
				m_type(type) {}

			void* new_component(usize& index)
			{
				usize size = get_type_size(m_type);
				if (m_size >= m_capacity)
				{
					usize align = get_type_alignment(m_type);
					usize new_cap = max<usize>(m_capacity * 2, 1);
					void* new_buf = memalloc(new_cap * size, align);
					if (m_data)
					{
						relocate_type_range(m_type, new_buf, m_data, m_size);
						memfree(m_data, align);
					}
					m_data = new_buf;
					m_capacity = new_cap;
				}
				void* ret = (void*)((usize)m_data + size * m_size);
				index = m_size;
				++m_size;
				return ret;
			}
			~ComponentBuffer()
			{
				if (m_data)
				{
					if (m_size)
					{
						destruct_type_range(m_type, m_data, m_size);
					}
					memfree(m_data, get_type_alignment(m_type));
				}
			}
		};

		struct OpList
		{
			Vector<u8> m_op_data;

			template <typename _Ty>
			void write(const _Ty& data)
			{
				const u8* begin = reinterpret_cast<const u8*>(&data);
				m_op_data.insert(m_op_data.end(), Span<const u8>(begin, sizeof(_Ty)));
			}
		};

		struct ChangeListData
		{
			OpList m_ops;
			// Holds buffer to contain new components.
			// Every component will have one buffer.
			HashMap<typeinfo_t, ComponentBuffer> m_new_component_data;

			ChangeListData() {}

			ChangeListData(ChangeListData&& rhs) :
				m_ops(move(rhs.m_ops)),
				m_new_component_data(move(rhs.m_new_component_data)) {}

			void reset()
			{
				m_ops.m_op_data.clear();
				m_new_component_data.clear();
			}

			entity_id_t add_entity(entity_id_t id)
			{
				m_ops.write(ChangeListOpType::add_entity);
				m_ops.write(id);
				return id;
			}
			void remove_entity(entity_id_t id)
			{
				m_ops.write(ChangeListOpType::remove_entity);
				m_ops.write(id);
			}
			void remove_all_entities()
			{
				m_ops.write(ChangeListOpType::remove_all_entities);
			}
			void set_target_entity(entity_id_t id)
			{
				m_ops.write(ChangeListOpType::set_target_entity);
				m_ops.write(id);
			}
			void* add_component(typeinfo_t component_type, bool allow_overwrite, usize* data_index)
			{
				auto iter2 = m_new_component_data.find(component_type);
				if (iter2 == m_new_component_data.end())
				{
					iter2 = m_new_component_data.insert(make_pair(component_type, ComponentBuffer(component_type))).first;
				}
				usize index;
				void* data = iter2->second.new_component(index);
				if (data_index) *data_index = index;
				m_ops.write(allow_overwrite ? ChangeListOpType::add_component : ChangeListOpType::add_component_if_not_exists);
				m_ops.write(component_type);
				m_ops.write(index);
				return data;
			}
			void* set_component(typeinfo_t component_type, usize* data_index)
			{
				auto iter2 = m_new_component_data.find(component_type);
				if (iter2 == m_new_component_data.end())
				{
					iter2 = m_new_component_data.insert(make_pair(component_type, ComponentBuffer(component_type))).first;
				}
				usize index;
				void* data = iter2->second.new_component(index);
				if (data_index) *data_index = index;
				m_ops.write(ChangeListOpType::set_component);
				m_ops.write(component_type);
				m_ops.write(index);
				return data;
			}
			void* get_temp_component_data(typeinfo_t component_type, usize index)
			{
				auto iter2 = m_new_component_data.find(component_type);
				if (iter2 == m_new_component_data.end()) return nullptr;
				return (void*)((usize)iter2->second.m_data + get_type_size(iter2->second.m_type) * index);
			}
			void remove_component(typeinfo_t component_type)
			{
				m_ops.write(ChangeListOpType::remove_component);
				m_ops.write(component_type);
			}
			void remove_all_components()
			{
				m_ops.write(ChangeListOpType::remove_all_components);
			}
			void add_tag(entity_id_t tag)
			{
				m_ops.write(ChangeListOpType::add_tag);
				m_ops.write(tag);
			}
			void remove_tag(entity_id_t tag)
			{
				m_ops.write(ChangeListOpType::remove_tag);
				m_ops.write(tag);
			}
			void remove_all_tags()
			{
				m_ops.write(ChangeListOpType::remove_all_tags);
			}
		};
	}
}