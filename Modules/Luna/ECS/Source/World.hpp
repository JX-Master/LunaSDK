/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.hpp
* @author JXMaster
* @date 2022/8/11
*/
#pragma once
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Mutex.hpp>
#include "ChangeListData.hpp"
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include "Cluster.hpp"
#include <Luna/Runtime/SelfIndexedHashMap.hpp>

namespace Luna
{
	namespace ECS
	{
		struct EntityRecord
		{
			Cluster* m_cluster;
			usize m_index;
			u32 m_generation;
		};

		struct EntityIdAllocator
		{
			RingDeque<entity_id_t> m_free_ids;
			u32 m_next_free_slot;
			SpinLock m_lock;

			EntityIdAllocator() :
				m_next_free_slot(0) {}

			entity_id_t allocate_id()
			{
				LockGuard guard(m_lock);
				if (!m_free_ids.empty())
				{
					entity_id_t id = m_free_ids.front();
					m_free_ids.pop_front();
					++id.generation;
					return id;
				}
				entity_id_t ret;
				ret.index = m_next_free_slot;
				ret.generation = 1;
				++m_next_free_slot;
				return ret;
			}

			void free_id(entity_id_t id)
			{
				LockGuard guard(m_lock);
				m_free_ids.push_back(id);
			}
		};

		enum class TaskType : u8
		{
			task,
			task_barrier,
		};

		struct TaskScheduleData
		{
			JobSystem::job_id_t m_id;
			HashSet<typeinfo_t> m_read_components;
			HashSet<typeinfo_t> m_write_components;
		};

		struct ClusterType
		{
			//! The sorted span that refers to all components in the archetype.
			Span<const typeinfo_t> components;
			//! The sorted span that refers to all tags in the archetype.
			Span<const entity_id_t> tags;

			bool operator==(const ClusterType& rhs) const
			{
				return (components.size() == rhs.components.size()) &&
					(tags.size() == rhs.tags.size()) &&
					(!memcmp(components.data(), rhs.components.data(), components.size_bytes())) &&
					(!memcmp(tags.data(), rhs.tags.data(), tags.size_bytes()));
			}

		};

		struct ClusterExtractKey
		{
			ClusterType operator()(const UniquePtr<Cluster>& value)
			{
				ClusterType r;
				r.components = { value->m_component_types.data(), value->m_component_types.size() };
				r.tags = { value->m_tags.data(), value->m_tags.size() };
				return r;
			}
		};

		struct World : public IWorld
		{
			lustruct("ECS::World", "{945066F9-0292-46DC-8659-41D1C5874EA6}");
			luiimpl();

			//! Entity allocation and management.
			EntityIdAllocator m_entity_id_allocator;
			Vector<EntityRecord> m_entities;

			//! Achetype for empty entity.
			Cluster* m_empty_cluster;

			//! Clusters managed by this world.
			SelfIndexedHashMap<ClusterType, UniquePtr<Cluster>, ClusterExtractKey> m_clusters;

			//! Task management.
			RingDeque<TaskScheduleData> m_tasks;
			JobSystem::job_id_t m_last_exclusive_task;
			Vector<ChangeListData> m_change_lists;
			Ref<IMutex> m_queue_lock;

			World() :
				m_last_exclusive_task(JobSystem::INVALID_JOB_ID)
			{
				UniquePtr<Cluster> empty_cluster(memnew<Cluster>());
				m_empty_cluster = empty_cluster.get();
				m_clusters.insert(move(empty_cluster));
				m_queue_lock = new_mutex();
			}

			Cluster* get_cluster(Span<const typeinfo_t> components, Span<const entity_id_t> tags,
				bool create_if_not_exist);
			EntityRecord* get_entity_record(entity_id_t id);
			EntityRecord* get_or_create_entity_record(entity_id_t id);

			void remove_finished_tasks();

			void add_entity_record(entity_id_t id);
			entity_id_t add_entity();
			void remove_entity(entity_id_t id);
			void remove_all_entities();
		};
	}

	template<>
	struct hash<ECS::ClusterType>
	{
		usize operator()(const ECS::ClusterType& val) const
		{
			usize h = 0;
			for (auto i : val.components)
			{
				h ^= hash<typeinfo_t>()(i);
			}
			for (auto i : val.tags)
			{
				h ^= hash<ECS::entity_id_t>()(i);
			}
			return h;
		}
	};
}