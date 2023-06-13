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
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/JobSystem/JobSystem.hpp>

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

			//! Gets the cluster by components and tags.
			 virtual Cluster* get_cluster(Span<const typeinfo_t> components, Span<const entity_id_t> tags, 
			 	bool create_if_not_exist = false) = 0;
		};

		//! Creates one new world.
		LUNA_ECS_API Ref<IWorld> new_world();
	}
}