/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Cluster.cpp
* @author JXMaster
* @date 2022/9/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "Cluster.hpp"
#include "World.hpp"

namespace Luna
{
	namespace ECS
	{
		usize Cluster::allocate_entry()
		{
			if (m_size == m_entities.size()) expand_buffer();
			usize r = m_size;
			++m_size;
			// All data remain unconstructed.
			return r;
		}
		void Cluster::free_entry(World* world, usize index)
		{
			// Destruct components.
			for (usize i = 0; i < m_component_types.size(); ++i)
			{
				typeinfo_t type = m_component_types[i];
				if (!is_type_trivially_destructable(type))
				{
					usize strip = get_type_size(type);
					void* data = (void*)((usize)(m_components[i]) + strip * index);
					destruct_type(type, data);
				}
			}
			--m_size;
			if (index != m_size)
			{
				// Swap the back entity to fill the empty space.
				relocate_entity(index, m_size);
				// Update world record for the swapped entity.
				auto& ent = world->m_entities[m_entities[index].index];
				ent.m_index = index;
			}
		}
		void Cluster::free_all_entities()
		{
			// Destruct components.
			for (usize i = 0; i < m_component_types.size(); ++i)
			{
				typeinfo_t type = m_component_types[i];
				if (!is_type_trivially_destructable(type))
				{
					destruct_type_range(type, m_components[i], m_size);
				}
			}
			m_entities.clear();
			m_size = 0;
		}
		void Cluster::relocate_entity(usize dst, usize src)
		{
			m_entities[dst] = move(m_entities[src]);
			for (usize i = 0; i < m_component_types.size(); ++i)
			{
				typeinfo_t type = m_component_types[i];
				usize strip = get_type_size(type);
				void* data = m_components[i];
				void* dst_ptr = (void*)((usize)(data) + strip * dst);
				void* src_ptr = (void*)((usize)(data) + strip * src);
				relocate_type(type, dst_ptr, src_ptr);
			}
		}
		void Cluster::expand_buffer()
		{
			// Expand from 1 to handle single-component case.
			usize old_capacity = m_entities.size();
			usize new_capacity = max<usize>(old_capacity * 2, 1);
			usize expand_count = new_capacity - old_capacity;
			// Resize entity.
			m_entities.resize(new_capacity);
			// Resize component arrays.
			for (usize i = 0; i < m_component_types.size(); ++i)
			{
				typeinfo_t type = m_component_types[i];
				void* old_data = m_components[i];
				usize type_size = get_type_size(type);
				void* new_data = memalloc(type_size * new_capacity, get_type_alignment(type));
				if (old_data)
				{
					relocate_type_range(type, new_data, old_data, m_size);
					memfree(old_data);
				}
				m_components[i] = new_data;
			}
		}
		LUNA_ECS_API Span<const typeinfo_t> get_cluster_components(Cluster* cluster)
		{
			return { cluster->m_component_types.data(), cluster->m_component_types.size() };
		}
		LUNA_ECS_API Span<const entity_id_t> get_cluster_tags(Cluster* cluster)
		{
			return { cluster->m_tags.data(), cluster->m_tags.size() };
		}
		LUNA_ECS_API void* get_cluster_components_data(Cluster* cluster, typeinfo_t component_type)
		{
			auto& component_types = cluster->m_component_types;
			auto iter = binary_search_iter(component_types.begin(), component_types.end(), component_type);
			if (iter == component_types.end()) return nullptr;
			usize index = iter - component_types.begin();
			return cluster->m_components[index];
		}
		LUNA_ECS_API void** get_cluster_components_data_array(Cluster* cluster)
		{
			return cluster->m_components;
		}
		LUNA_ECS_API Span<const entity_id_t> get_cluster_entities(Cluster* cluster)
		{
			return { cluster->m_entities.data(), cluster->m_size };
		}
	}
}