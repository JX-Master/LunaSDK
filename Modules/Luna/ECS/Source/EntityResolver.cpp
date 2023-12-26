
/*!
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
        usize relocate_entity(World* world, Cluster* src_cluster, usize src_index, Cluster* dst_cluster, const HashMap<typeinfo_t, void*>& data)
		{
			// allocate entity.
			usize dst_index = dst_cluster->allocate_entry();
			// move entity ID.
			auto& src_entity = src_cluster->m_entities[src_index];
			auto& dst_entity = dst_cluster->m_entities[dst_index];
			luassert(src_entity != NULL_ENTITY);
			dst_entity = move(src_entity);

			// Initialize component data.
			usize src_component_index = 0;
			usize dst_component_index = 0;
			auto src_components = src_cluster->m_component_types;
			auto dst_components = dst_cluster->m_component_types;

			while (src_component_index < src_components.size() && dst_component_index < dst_components.size())
			{
				typeinfo_t src_component_type = src_components[src_component_index];
				typeinfo_t dst_component_type = dst_components[dst_component_index];
				if (src_component_type == dst_component_type)
				{
					// relocate components.
					auto iter2 = data.find(dst_component_type);
					usize strip = get_type_size(dst_component_type);
					void* dst_data = (void*)((usize)dst_cluster->m_components[dst_component_index] + strip * dst_index);
					void* src_data = (iter2 != data.end()) ? iter2->second :
						(void*)((usize)(src_cluster->m_components[src_component_index]) + strip * src_index);
					move_construct_type(dst_component_type, dst_data, src_data);
					++src_component_index;
					++dst_component_index;
				}
				else if (src_component_type < dst_component_type)
				{
					// exist in src but not in dst, remove.
					++src_component_index;
				}
				else
				{
					// exist in dst but not in src, add.
					auto iter2 = data.find(dst_component_type);
					usize strip = get_type_size(dst_component_type);
					void* dst_data = (void*)((usize)dst_cluster->m_components[dst_component_index] + strip * dst_index);
					if (iter2 != data.end())
					{
						void* src_data = iter2->second;
						move_construct_type(dst_component_type, dst_data, src_data);
					}
					else
					{
						construct_type(dst_component_type, dst_data);
					}
					++dst_component_index;
				}
			}
			while (src_component_index < src_components.size())
			{
				// exist in src but not in dst, remove.
				++src_component_index;
			}
			while (dst_component_index < dst_components.size())
			{
				// exist in dst but not in src, add.
				typeinfo_t dst_component_type = dst_components[dst_component_index];
				auto iter2 = data.find(dst_component_type);
				usize strip = get_type_size(dst_component_type);
				void* dst_data = (void*)((usize)dst_cluster->m_components[dst_component_index] + strip * dst_index);
				if (iter2 != data.end())
				{
					void* src_data = iter2->second;
					move_construct_type(dst_component_type, dst_data, src_data);
				}
				else
				{
					construct_type(dst_component_type, dst_data);
				}
				++dst_component_index;
			}
			
			// remove old entity data.
			src_cluster->free_entry(world, src_index);
			return dst_index;
		}
        
        void EntityResolver::apply(World* world, entity_id_t entity)
		{
			// Alter record.
			EntityRecord* record = world->get_entity_record(entity);
			if (record)
			{
				Cluster* dst_cluster = world->get_cluster(
					{m_component_types.data(), m_component_types.size()},
					{ m_tags.data(), m_tags.size() }, true);
				usize dst_index = m_src_index;
				if (dst_cluster != m_src_cluster)
				{
					usize dst_index = relocate_entity(world, m_src_cluster, m_src_index, dst_cluster, m_data);
					// update record.
					record->m_index = dst_index;
					record->m_cluster = dst_cluster;
				}
			}
			m_data.clear();
		}
        bool EntityResolver::add_component(typeinfo_t component)
		{
			auto dst_components_begin = (typeinfo_t*)alloca(sizeof(typeinfo_t) * (m_component_types.size() + 1));
			auto dst_components_end = set_union(
				m_component_types.begin(), m_component_types.end(),
				&component, &component + 1,
				dst_components_begin);
			usize new_size = (usize)(dst_components_end - dst_components_begin);
			if (new_size != m_component_types.size())
			{
				m_component_types.assign(Span<typeinfo_t>(dst_components_begin, new_size));
				return true;
			}
			return false;
		}
		bool EntityResolver::remove_component(typeinfo_t component)
		{
			auto dst_components_begin = (typeinfo_t*)alloca(sizeof(typeinfo_t) * m_component_types.size());
			auto dst_components_end = set_difference(
				m_component_types.begin(), m_component_types.end(),
				&component, &component + 1,
				dst_components_begin);
			usize new_size = (usize)(dst_components_end - dst_components_begin);
			if (new_size != m_component_types.size())
			{
				m_component_types.assign(Span<typeinfo_t>(dst_components_begin, new_size));
				return true;
			}
			return false;
		}
		bool EntityResolver::add_tag(entity_id_t tag)
		{
			auto dst_tags_begin = (entity_id_t*)alloca(sizeof(entity_id_t) * (m_tags.size() + 1));
			auto dst_tags_end = set_union(
				m_tags.begin(), m_tags.end(),
				&tag, &tag + 1,
				dst_tags_begin);
			usize new_size = (usize)(dst_tags_end - dst_tags_begin);
			if (new_size != m_tags.size())
			{
				m_tags.assign(Span<entity_id_t>(dst_tags_begin, new_size));
				return true;
			}
			return false;
		}
		bool EntityResolver::remove_tag(entity_id_t tag)
		{
			auto dst_tags_begin = (entity_id_t*)alloca(sizeof(entity_id_t) * m_tags.size());
			auto dst_tags_end = set_difference(
				m_tags.begin(), m_tags.end(),
				&tag, &tag + 1,
				dst_tags_begin);
			usize new_size = (usize)(dst_tags_end - dst_tags_begin);
			if (new_size != m_tags.size())
			{
				m_tags.assign(Span<entity_id_t>(dst_tags_begin, new_size));
				return true;
			}
			return false;
		}
    }
}