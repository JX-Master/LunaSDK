
/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EntityResolver.cpp
* @author JXMaster
* @date 2023/1/10
*/
#include "EntityResolver.hpp"
#include "World.hpp"

namespace Luna
{
    namespace ECS
    {
        usize relocate_entity(World* world, Cluster* src_cluster, usize src_index, Cluster* dest_cluster, const HashMap<typeinfo_t, void*>& data)
		{
			// allocate entity.
			usize dest_index = dest_cluster->allocate_entry();
			// move entity ID.
			auto& src_entity = src_cluster->m_entities[src_index];
			auto& dest_entity = dest_cluster->m_entities[dest_index];
			luassert(src_entity != NULL_ENTITY);
			dest_entity = move(src_entity);

			// Initialize component data.
			usize src_component_index = 0;
			usize dest_component_index = 0;
			auto src_components = src_cluster->m_component_types;
			auto dest_components = dest_cluster->m_component_types;

			while (src_component_index < src_components.size() && dest_component_index < dest_components.size())
			{
				typeinfo_t src_component_type = src_components[src_component_index];
				typeinfo_t dest_component_type = dest_components[dest_component_index];
				if (src_component_type == dest_component_type)
				{
					// relocate components.
					auto iter2 = data.find(dest_component_type);
					usize strip = get_type_size(dest_component_type);
					void* dest_data = (void*)((usize)dest_cluster->m_components[dest_component_index] + strip * dest_index);
					void* src_data = (iter2 != data.end()) ? iter2->second :
						(void*)((usize)(src_cluster->m_components[src_component_index]) + strip * src_index);
					move_construct_type(dest_component_type, dest_data, src_data);
					++src_component_index;
					++dest_component_index;
				}
				else if (src_component_type < dest_component_type)
				{
					// exist in src but not in dest, remove.
					++src_component_index;
				}
				else
				{
					// exist in dest but not in src, add.
					auto iter2 = data.find(dest_component_type);
					usize strip = get_type_size(dest_component_type);
					void* dest_data = (void*)((usize)dest_cluster->m_components[dest_component_index] + strip * dest_index);
					if (iter2 != data.end())
					{
						void* src_data = iter2->second;
						move_construct_type(dest_component_type, dest_data, src_data);
					}
					else
					{
						construct_type(dest_component_type, dest_data);
					}
					++dest_component_index;
				}
			}
			while (src_component_index < src_components.size())
			{
				// exist in src but not in dest, remove.
				++src_component_index;
			}
			while (dest_component_index < dest_components.size())
			{
				// exist in dest but not in src, add.
				typeinfo_t dest_component_type = dest_components[dest_component_index];
				auto iter2 = data.find(dest_component_type);
				usize strip = get_type_size(dest_component_type);
				void* dest_data = (void*)((usize)dest_cluster->m_components[dest_component_index] + strip * dest_index);
				if (iter2 != data.end())
				{
					void* src_data = iter2->second;
					move_construct_type(dest_component_type, dest_data, src_data);
				}
				else
				{
					construct_type(dest_component_type, dest_data);
				}
				++dest_component_index;
			}
			
			// remove old entity data.
			src_cluster->free_entry(world, src_index);
			return dest_index;
		}
        
        void EntityResolver::apply(World* world, entity_id_t entity)
		{
			// Alter record.
			EntityRecord* record = world->get_entity_record(entity);
			if (record)
			{
				Cluster* dest_cluster = world->get_cluster(
					{m_component_types.data(), m_component_types.size()},
					{ m_tags.data(), m_tags.size() }, true);
				usize dest_index = m_src_index;
				if (dest_cluster != m_src_cluster)
				{
					usize dest_index = relocate_entity(world, m_src_cluster, m_src_index, dest_cluster, m_data);
					// update record.
					record->m_index = dest_index;
					record->m_cluster = dest_cluster;
				}
			}
			m_data.clear();
		}
        bool EntityResolver::add_component(typeinfo_t component)
		{
			auto dest_components_begin = (typeinfo_t*)alloca(sizeof(typeinfo_t) * (m_component_types.size() + 1));
			auto dest_components_end = set_union(
				m_component_types.begin(), m_component_types.end(),
				&component, &component + 1,
				dest_components_begin);
			usize new_size = (usize)(dest_components_end - dest_components_begin);
			if (new_size != m_component_types.size())
			{
				m_component_types.assign_n(dest_components_begin, new_size);
				return true;
			}
			return false;
		}
		bool EntityResolver::remove_component(typeinfo_t component)
		{
			auto dest_components_begin = (typeinfo_t*)alloca(sizeof(typeinfo_t) * m_component_types.size());
			auto dest_components_end = set_difference(
				m_component_types.begin(), m_component_types.end(),
				&component, &component + 1,
				dest_components_begin);
			usize new_size = (usize)(dest_components_end - dest_components_begin);
			if (new_size != m_component_types.size())
			{
				m_component_types.assign_n(dest_components_begin, new_size);
				return true;
			}
			return false;
		}
		bool EntityResolver::add_tag(entity_id_t tag)
		{
			auto dest_tags_begin = (entity_id_t*)alloca(sizeof(entity_id_t) * (m_tags.size() + 1));
			auto dest_tags_end = set_union(
				m_tags.begin(), m_tags.end(),
				&tag, &tag + 1,
				dest_tags_begin);
			usize new_size = (usize)(dest_tags_end - dest_tags_begin);
			if (new_size != m_tags.size())
			{
				m_tags.assign_n(dest_tags_begin, new_size);
				return true;
			}
			return false;
		}
		bool EntityResolver::remove_tag(entity_id_t tag)
		{
			auto dest_tags_begin = (entity_id_t*)alloca(sizeof(entity_id_t) * m_tags.size());
			auto dest_tags_end = set_difference(
				m_tags.begin(), m_tags.end(),
				&tag, &tag + 1,
				dest_tags_begin);
			usize new_size = (usize)(dest_tags_end - dest_tags_begin);
			if (new_size != m_tags.size())
			{
				m_tags.assign_n(dest_tags_begin, new_size);
				return true;
			}
			return false;
		}
    }
}