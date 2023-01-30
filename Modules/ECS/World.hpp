/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.hpp
* @author JXMaster
* @date 2023/1/4
*/
#pragma once
#include "Cluster.hpp"
#include <Runtime/Interface.hpp>
#include <Runtime/Ref.hpp>
#include <Runtime/Result.hpp>
#include <JobSystem/JobSystem.hpp>

namespace Luna
{
	namespace ECS
	{
		enum class TaskBarrierFlag : u8
		{
			none = 0x00,
			//! Executes all change lists submitted to the world in this barrier.
			flush_change_lists = 0x01,
		};

		enum class TaskFlag : u8
		{
			none = 0x00,
			//! This task should be run exclusively, no other tasks can be run in parallel.
			//! This is required if this task will perform structural changes (adding/removing entities and components), or
			//! if the components this task will access are variable.
			exclusive = 0x01,
		};

		struct IWorld;

		using task_func_t = void(IWorld* world, void* params);

		struct TaskDesc
		{
			Span<typeinfo_t> read_components;
			Span<typeinfo_t> write_components;
			TaskFlag flags = TaskFlag::none;
		};

		//! @interface IWorld
		//! Represents one ECS context that holds entities and their components. Every world is independent to each other.
		//! @remark The world object implements `IChangeList` as well. In such case, all calls to `IChangeList` behave like being committed immediately 
		//! before return.
		//! The world itself is not thread safe, the user must ensure that modifications to the world are synchronized.
		struct IWorld : virtual Interface
		{
			luiid("{14F85B5E-D509-40A8-A7F6-49778783418A}");

			//! Gets the entity record for the specified entity.
			// virtual RV query_entity(entity_id_t id, EntityAddress* out_address) = 0;

			// bool is_entity_valid(entity_id_t id)
			// {
			// 	return succeeded(query_entity(id, nullptr));
			// }

			// template <typename _Ty>
			// bool has_component(entity_id_t id)
			// {
			// 	Cluster* cluster;
			// 	usize index;
			// 	auto r = query_entity(id, &cluster, &index);
			// 	if (failed(r)) return r.errcode();
			// 	typeinfo_t type = typeof<_Ty>();
			// 	void* components = get_cluster_components_data(cluster, type);
			// 	return components ? true : false;
			// }

			// template <typename _Ty>
			// R<_Ty*> get_component(entity_id_t id)
			// {
			// 	Cluster* cluster;
			// 	usize index;
			// 	auto r = query_entity(id, &cluster, &index);
			// 	if (failed(r)) return r.errcode();
			// 	typeinfo_t type = typeof<_Ty>();
			// 	void* components = get_cluster_components_data(cluster, type);
			// 	if (!components) return ECSError::component_not_found();
			// 	return (_Ty*)((usize)components + index * get_type_size(type));
			// }

			//! Gets the cluster by components and tags.
			 virtual Cluster* get_cluster(Span<const typeinfo_t> components, Span<const entity_id_t> tags, 
			 	bool create_if_not_exist = false) = 0;

			// //! Adds one entity to the world. This modification is applied immediately.
			// //! @return Returns the entity ID of the created entity.
			// //! @remark This function can only be called when the caller has exclusive access to the world.
			// virtual entity_id_t add_entity() = 0;

			// //! Removes the specified entity from the world. This modification is applied immediately.
			// //! @param[in] id The ID of the entity to remove.
			// //! @remark If the entity does not exist when this operation is committed, this operation does nothing.
			// //! This function can only be called when the caller has exclusive access to the world.
			// virtual void remove_entity(entity_id_t id) = 0;

			// //! Removes all entities in the world. This modification is applied immediately.
			// //! @remark This function can only be called when the caller has exclusive access to the world.
			// virtual void remove_all_entities() = 0;

			// //! Adds and removes multiple components and tags by specifying the target cluster directly.
			// //! @remark This function can only be called when the caller has exclusive access to the world.
			// virtual void set_entity_cluster(const EntityAddress& src_address, 
			// 	Cluster* dest_cluster,
			// 	const HashMap<typeinfo_t, void*>& data,
			// 	EntityAddress* out_address) = 0;

			// //! Adds one task barrier to the processing queue.
			// virtual JobSystem::job_id_t enqueue_task_barrier(TaskBarrierFlag flags) = 0;

			// //! Adds one task to the processing queue.
			// virtual	JobSystem::job_id_t enqueue_task(const TaskDesc& desc, task_func_t* task_func, void* params = nullptr) = 0;

			// //! Begins a synchronized task on the current thread.
			// //! @param[in] desc The task descriptor.
			// //! @return Returns the job ID that should be passed to `end_task` when the task is finished.
			// virtual JobSystem::job_id_t begin_task(const TaskDesc& desc) = 0;

			// //! Finishes the task opend by `begin_task`.
			// //! @param[in] task The job ID returned by `begin_task`.
			// virtual void end_task(JobSystem::job_id_t task) = 0;
		};

		//! Creates one new world.
		LUNA_ECS_API Ref<IWorld> new_world();
	}
}