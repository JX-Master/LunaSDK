/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TaskContext.hpp
* @author JXMaster
* @date 2023/1/6
*/
#pragma once
#include "Cluster.hpp"
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>

namespace Luna
{
	namespace ECSError
	{
		LUNA_ECS_API errcat_t errtype();
		LUNA_ECS_API ErrCode entity_not_found();
		LUNA_ECS_API ErrCode component_not_found();
	}

    namespace ECS
    {
        enum class TaskExecutionMode : u8
        {
            //! This task can run concurrently with other tasks based on read and write components specified by the task.
			//! 
			//! The component types that can be read from and written to by this task is specified by `read_components` and `write_components` 
			//! when `ITaskContext::begin` is called.
			//! 
			//! Structural changes (adding/removing entities, components and tags) will be cached and applied to the world in another trailing 
			//! exclusive task when `end` is called.
            shared = 0,
            //! This task should have exclusive access to the world, no other tasks can run concurrently.
			//!
			//! The task have read and write access to all component types. `read_components` and `write_components` in `ITaskContext::begin` are ignored. 
			//! All modifications (including structural changes) are written to the world immediately. No cache is used. 
            exclusive = 1,
        };

        struct IWorld;

		using filter_func_t = bool(Cluster* cluster, void* userdata);

		namespace Impl
		{
			template <typename _Ty>
			bool filter_invoker(Cluster* cluster, void* userdata)
			{
				_Ty* filter = (_Ty*)userdata;
				return (*filter)(cluster);
			}
		}

        //! Used by task to read and write world data.
        struct ITaskContext : virtual Interface
        {
            luiid("{1ef50f4a-f3df-439c-815f-bf767a3e98d7}");

            //! Resets the task context and begins a new task.
            //! @param[in] world The world to run the task on.
            //! @param[in] exec_mode The task execution mode.
            //! @param[in] read_components When `exec_mode` is `shared`, specify components that 
			//! will be read from by this task.
            //! @param[in] write_components When `exec_mode` is `shared`, specify components that will be read from and 
            //! written to by this task.
            //! @remark This call may block the current thread until all components required by this task can be safely
            //! accessed by this task, or until all other tasks are finished if this is a exclusive task.
            virtual void begin(
                IWorld* world,
                TaskExecutionMode exec_mode,
                Span<typeinfo_t> read_components,
                Span<typeinfo_t> write_components
                ) = 0;

            //! Gets the world which this task context is attached to.
			virtual IWorld* get_world() = 0;

            //! Gets the entity address for the specified entity.
			virtual R<EntityAddress> get_entity(entity_id_t id) = 0;

			virtual void get_clusters(Vector<Cluster*>& result, filter_func_t* filter, void* userdata) = 0;

			template <typename _Filter>
			void get_clusters(Vector<Cluster*>& result, _Filter&& filter)
			{
				get_clusters(result, Impl::filter_invoker<_Filter>, (void*)addressof(filter));
			}

            bool is_entity_valid(entity_id_t id)
			{
				return succeeded(get_entity(id));
			}

            template <typename _Ty>
			bool has_component(entity_id_t id)
			{
				auto r = get_entity(id);
				if (failed(r)) return false;
				typeinfo_t type = typeof<_Ty>();
                auto components = get_cluster_components(r.get().cluster);
                return binary_search(components.begin(), components.end(), type);
			}

			template <typename _Ty>
			R<_Ty*> get_component(entity_id_t id)
			{
				auto r = get_entity(id);
				if (failed(r)) return r.errcode();
				typeinfo_t type = typeof<_Ty>();
				void* components = get_cluster_components_data(r.get().cluster, type);
				if (!components) return ECSError::component_not_found();
				return (_Ty*)((usize)components + r.get().index * get_type_size(type));
			}

            //! Adds one entity to the world.
			//! @return Returns the entity ID of the created entity.
			virtual entity_id_t add_entity() = 0;

            //! Removes the specified entity from the world.
			//! @param[in] id The ID of the entity to remove.
			//! @remark If the entity does not exist when this operation is committed, this operation does nothing.
			virtual void remove_entity(entity_id_t id) = 0;

            //! Removes all entities in the world.
			virtual void remove_all_entities() = 0;

			//! Changes the target entity for succeeding component and tag modification calls, including:
			//! * `add_component`
			//! * `remove_component`
			//! * `set_component`
			//! * `remove_all_components` 
			//! * `add_tag`
			//! * `remove_tag`
			//! * `remove_all_tags`
			virtual void set_target_entity(entity_id_t id) = 0;

            //! Adds one component to the target entity.
			//! @param[in] component_type The type of the component to add.
			//! @param[in] allow_overwrite If this is `true` and the specified component already exists, this call overwrites the old data 
			//! with the new data. If this is `false` and the specified component already exists, this call does nothing.
			//! @param[out] data_index If specified, returns the data index which can be used to query data pointer at a future time.
			//! @return Returns the pointer to the new component data. 
			//! The pointer is valid until another component add call is issued on the same change list.
			//! The returned component data is uninitialized, it is user's responsibility to initialize the component data before committing
			//! this change list. The system does not track the component state, committing uninitialized component may result in undefined hehavior.
			virtual void* add_component(typeinfo_t component_type, bool allow_overwrite = false, usize* data_index = nullptr) = 0;

            template<typename _Ty>
			_Ty* add_component(bool allow_overwrite = false, usize* data_index = nullptr)
			{
				return (_Ty*)add_component(typeof<_Ty>(), allow_overwrite, data_index);
			}

            //! Gets the component data being added by `add_component`.
			//! @param[in] component_type The component type to get.
			//! @param[in] index The index of the component fetched from `add_component`.
			//! @return Returns the pointer to the component data.
			//! The pointer is valid until another component add call is issued on the same change list.
			virtual void* get_temp_component_data(typeinfo_t component_type, usize index) = 0;

			template <typename _Ty>
			_Ty* get_temp_component_data(usize index)
			{
				return (_Ty*)get_temp_component_data(typeof<_Ty>(), index);
			}

            //! Removes the specified component.
			virtual void remove_component(typeinfo_t component_type) = 0;

            template <typename _Ty>
			void remove_component()
			{
				remove_component(typeof<_Ty>());
			}

            //! Removes all components of the target entity.
			virtual void remove_all_components() = 0;

            //! Adds one tag to the target entity.
			//! Tags are entity ids that is used to group and filter entities.
			virtual void add_tag(entity_id_t tag) = 0;

			//! Removes one id from the target entity.
			virtual void remove_tag(entity_id_t tag) = 0;

			//! Removes all ids of the target entity.
			virtual void remove_all_tags() = 0;

            //! Finishes the current task and lets succeeding tasks to be run.
            virtual void end() = 0;
        };

		LUNA_ECS_API Ref<ITaskContext> new_task_context();
    }   
}
