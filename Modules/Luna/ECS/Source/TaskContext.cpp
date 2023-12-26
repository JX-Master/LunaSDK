
/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TaskContext.cpp
* @author JXMaster
* @date 2023/1/10
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ECS_API LUNA_EXPORT
#include "TaskContext.hpp"
#include "World.hpp"
#include "EntityResolver.hpp"
#include <Luna/Runtime/Log.hpp>

namespace Luna
{
    namespace ECS
    {
        template <typename _Ty>
		inline void read_data(_Ty& value, u8*& data_ptr)
		{
			memcpy(&value, data_ptr, sizeof(_Ty));
			data_ptr += sizeof(_Ty);
		}

        inline EntityResolver* get_resolver(World* world, HashMap<entity_id_t, EntityResolver>& resolvers, 
            entity_id_t id)
        {
            auto iter = resolvers.find(id);
            if(iter == resolvers.end())
            {
                auto record = world->get_entity_record(id);
                if (!record)
				{
					log_warning("ECS", "TaskContext::end - Invalid entity ID was specified in the context, the entity may be removed or not created. All operations to the entity will be ignored.");
                    return nullptr;
				}
                EntityResolver resolver;
                resolver.m_src_cluster = record->m_cluster;
                resolver.m_src_index = record->m_index;
                resolver.m_component_types.assign(record->m_cluster->m_component_types);
				resolver.m_tags.assign(record->m_cluster->m_tags);
                iter = resolvers.insert(make_pair(id, move(resolver))).first;
            }
            return &(iter->second);
        }
        void TaskContext::apply_change_list()
        {
            u8* data_ptr = m_data.m_ops.m_op_data.data();
			u8* data_end = data_ptr + m_data.m_ops.m_op_data.size();
            HashMap<entity_id_t, EntityResolver> resolvers;
            EntityResolver* current_resolver = nullptr;
            while (data_ptr < data_end)
			{
                ChangeListOpType op;
				read_data(op, data_ptr);
                switch(op)
                {
                case ChangeListOpType::add_entity:
                {
                    entity_id_t id;
                    read_data(id, data_ptr);
                    m_world->add_entity_record(id);
                }
                break;
                case ChangeListOpType::remove_entity:
                {
                    entity_id_t id;
                    read_data(id, data_ptr);
                    m_world->remove_entity(id);
                }
                break;
                case ChangeListOpType::remove_all_entities:
                {
                    m_world->remove_all_entities();
                }
                break;
                case ChangeListOpType::set_target_entity:
                {
                    entity_id_t id;
				    read_data(id, data_ptr);
                    current_resolver = get_resolver(m_world, resolvers, id);
                }
                break;
                case ChangeListOpType::add_component:
                case ChangeListOpType::add_component_if_not_exists:
                {
                    typeinfo_t component_type;
				    usize index;
                    read_data(component_type, data_ptr);
				    read_data(index, data_ptr);
                    if(current_resolver)
                    {
                        bool added = current_resolver->add_component(component_type);
                        // If the component is actually added, or the user need to add component even if exists.
                        if (added || op == ChangeListOpType::add_component)
                        {
                            auto iter = m_data.m_new_component_data.find(component_type);
                            luassert(iter != m_data.m_new_component_data.end());
                            void* ptr = (void*)((usize)iter->second.m_data + index * get_type_size(iter->second.m_type));
                            current_resolver->m_data.insert_or_assign(component_type, ptr);
                        }
                    }
                }
                break;
                case ChangeListOpType::remove_component:
                {
                    typeinfo_t component_type;
                    read_data(component_type, data_ptr);
                    if (current_resolver)
                    {
                        current_resolver->remove_component(component_type);
                        current_resolver->m_data.erase(component_type);
                    }
                }
                break;
                case ChangeListOpType::remove_all_components:
                {
                    if (current_resolver)
                    {
                        current_resolver->m_component_types.clear();
                        current_resolver->m_data.clear();
                    }
                }
                break;
                case ChangeListOpType::add_tag:
                {
                    entity_id_t tag;
                    read_data(tag, data_ptr);
                    if (current_resolver)
                    {
                        current_resolver->add_tag(tag);
                    }
                }
                break;
                case ChangeListOpType::remove_tag:
                {
                    entity_id_t tag;
                    read_data(tag, data_ptr);
                    if (current_resolver)
                    {
                        current_resolver->remove_tag(tag);
                    }
                }
                break;
                case ChangeListOpType::remove_all_tags:
                {
                    if (current_resolver)
                    {
                        current_resolver->m_tags.clear();
                    }
                }
                break;
                default: lupanic();
                }
            }
            for(auto& resolver : resolvers)
            {
                resolver.second.apply(m_world, resolver.first);
            }
        }
        JobSystem::job_id_t TaskContext::begin_task(
                TaskExecutionMode exec_mode,
                Span<typeinfo_t> read_components,
                Span<typeinfo_t> write_components)
        {
            MutexGuard guard(m_world->m_queue_lock);
            auto id = JobSystem::allocate_job_id();
            Vector<JobSystem::job_id_t> wait_jobs;
            if (m_world->m_last_exclusive_task != JobSystem::INVALID_JOB_ID)
			{
				wait_jobs.push_back(m_world->m_last_exclusive_task);
			}
            m_world->remove_finished_tasks();
			if (exec_mode == TaskExecutionMode::exclusive)
			{
                // Wait for all previous tasks.
				for (auto& i : m_world->m_tasks)
				{
					wait_jobs.push_back(i.m_id);
				}
                // Clear all tasks.
				m_world->m_tasks.clear();
				m_world->m_last_exclusive_task = id;
			}
			else
			{
				TaskScheduleData data;
				for (usize i = 0; i < read_components.size(); ++i)
				{
					data.m_read_components.insert(read_components[i]);
				}
				for (usize i = 0; i < write_components.size(); ++i)
				{
					data.m_write_components.insert(write_components[i]);
				}
				// Resolve wait tasks.
				for (auto& t : m_world->m_tasks)
				{
					bool wait_this_task = false;
					// Block write for reads.
					for (typeinfo_t i : data.m_read_components)
					{
						if (t.m_write_components.contains(i))
						{
							wait_this_task = true;
							break;
						}
					}
					if (!wait_this_task)
					{
						// Block read & write for writes.
						for (typeinfo_t i : data.m_write_components)
						{
							if (t.m_read_components.contains(i) || t.m_write_components.contains(i))
							{
								wait_this_task = true;
								break;
							}
						}
					}
					if (wait_this_task)
					{
						wait_jobs.push_back(t.m_id);
					}
				}
				data.m_id = id;
				m_world->m_tasks.push_back(move(data));
			}
			guard.unlock();
            // Waits for all dependency tasks before running this task.
			for (auto job : wait_jobs)
			{
				JobSystem::wait_job(job);
			}
            return id;
        }
        void TaskContext::end_task(JobSystem::job_id_t id)
        {
            JobSystem::finish_job_id(m_job_id);
        }

        void TaskContext::begin(IWorld* world,
                TaskExecutionMode exec_mode,
                Span<typeinfo_t> read_components,
                Span<typeinfo_t> write_components)
        {
            m_world = (World*)world->get_object();
            m_exec_mode = exec_mode;
            m_job_id = begin_task(exec_mode, read_components, write_components);
        }
        void TaskContext::end()
        {
            if(!m_data.m_ops.m_op_data.empty())
            {
                if(m_exec_mode == TaskExecutionMode::shared)
                {
                    end_task(m_job_id);
                    // Starts an another exclusive context to apply all changes.
                    auto id = begin_task(TaskExecutionMode::exclusive, {}, {});
                    apply_change_list();
                    end_task(id);
                }
                else
                {
                    apply_change_list();
                    end_task(m_job_id);
                }
            }
            m_data.reset();
        }
        R<EntityAddress> TaskContext::get_entity(entity_id_t id)
        {
            EntityRecord* record = m_world->get_entity_record(id);
			if (!record) return ECSError::entity_not_found();
            EntityAddress ret;
            ret.cluster = record->m_cluster;
            ret.index = record->m_index;
			return ret;
        }
        void TaskContext::get_clusters(Vector<Cluster*>& result, filter_func_t* filter, void* userdata)
        {
            result.clear();
            for(auto& cluster : m_world->m_clusters)
            {
                if(filter(cluster.get(), userdata))
                {
                    result.push_back(cluster.get());
                }
            }
        }
        LUNA_ECS_API Ref<ITaskContext> new_task_context()
        {
            return new_object<TaskContext>();
        }
    }
}
