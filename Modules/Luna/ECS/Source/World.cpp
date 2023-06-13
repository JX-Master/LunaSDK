/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.cpp
* @author JXMaster
* @date 2022/8/11
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "World.hpp"
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Log.hpp>
namespace Luna
{
	namespace ECS
	{
		Cluster* World::get_cluster(Span<const typeinfo_t> components, Span<const entity_id_t> tags,
			bool create_if_not_exist)
		{
			ClusterType cluster_type;
			cluster_type.components = components;
			cluster_type.tags = tags;
			// This is a slow process and the result should be cached.
			auto iter = m_clusters.find(cluster_type);
			if (iter != m_clusters.end())
			{
				return (*iter).get();
			}
			if (create_if_not_exist)
			{
				// Create new cluster.
				UniquePtr<Cluster> new_cluster{ memnew<Cluster>() };
				Cluster* ret = new_cluster.get();
				if (!components.empty())
				{
					typeinfo_t* component_types = (typeinfo_t*)memalloc(components.size_bytes());
					memcpy(component_types, components.data(), components.size_bytes());
					new_cluster->m_component_types = { component_types, components.size() };
				}
				if (!tags.empty())
				{
					entity_id_t* tags_buf = (entity_id_t*)memalloc(tags.size_bytes());
					memcpy(tags_buf, tags.data(), tags.size_bytes());
					new_cluster->m_tags = { tags_buf, tags.size() };
				}
				if (!components.empty())
				{
					new_cluster->m_components = (void**)memalloc(sizeof(void*) * components.size());
					memzero(new_cluster->m_components, sizeof(void*) * components.size());
				}
				m_clusters.insert(move(new_cluster));
				return ret;
			}
			return nullptr;
		}
		EntityRecord* World::get_entity_record(entity_id_t id)
		{
			if ((u64)id.index >= m_entities.size()) return nullptr;
			auto& ent = m_entities[id.index];
			if (ent.m_generation != id.generation || !ent.m_cluster) return nullptr;
			return &ent;
		}
		EntityRecord* World::get_or_create_entity_record(entity_id_t id)
		{
			while (id.index >= m_entities.size())
			{
				// Expand array.
				if (m_entities.size() == m_entities.capacity())
				{
					m_entities.reserve(m_entities.size() + 1024);
				}
				auto rec = m_entities.emplace_back();
				rec->m_cluster = nullptr;
				rec->m_generation = 0;
			}
			return &m_entities[id.index];
		}
		void World::remove_finished_tasks()
		{
			while (!m_tasks.empty())
			{
				auto& task = m_tasks.front();
				if (JobSystem::is_job_finished(task.m_id))
				{
					m_tasks.pop_front();
				}
				else
				{
					break;
				}
			}
		}
		void World::add_entity_record(entity_id_t id)
		{
			auto cluster = m_empty_cluster;
			auto index = cluster->allocate_entry();
			// Expand array if necessary.
			auto ent = get_or_create_entity_record(id);
			ent->m_generation = id.generation;
			ent->m_cluster = cluster;
			ent->m_index = index;
			// Initialize entity.
			cluster->m_entities[index] = id;
		}
		entity_id_t World::add_entity()
		{
			entity_id_t id = m_entity_id_allocator.allocate_id();
			add_entity_record(id);
			return id;
		}
		void World::remove_entity(entity_id_t id)
		{
			auto record = get_entity_record(id);
			if (!record)
			{
				log_warning("ECS", "World::remove_entity - Invalid entity ID: %llu, this function takes no effect.", id.value);
			}
			else
			{
				// Free entity data.
				record->m_cluster->free_entry(this, record->m_index);
				// Remove record.
				record->m_cluster = nullptr;
				m_entity_id_allocator.free_id(id);
			}
		}
		void World::remove_all_entities()
		{
			for (auto& cluster : m_clusters)
			{
				for (usize i = 0; i < cluster->m_size; ++i)
				{
					entity_id_t id = cluster->m_entities[i];
					auto record = get_entity_record(id);
					record->m_cluster = nullptr;
					m_entity_id_allocator.free_id(id);
				}
				cluster->free_all_entities();
			}
		}
		LUNA_ECS_API Ref<IWorld> new_world()
		{
			return new_object<World>();
		}
	}
}