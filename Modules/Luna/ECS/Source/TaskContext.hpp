
/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TaskContext.hpp
* @author JXMaster
* @date 2023/1/10
*/
#pragma once
#include "../TaskContext.hpp"
#include <Luna/Runtime/Ref.hpp>
#include <Luna/JobSystem/JobSystem.hpp>
#include "World.hpp"
namespace Luna
{
    namespace ECS
    {
        struct TaskContext : ITaskContext
        {
            lustruct("ECS::TaskContext", "{0da44741-176c-4fee-af5b-3938c84cd2b2}");
            luiimpl();
            
            Ref<World> m_world;
            JobSystem::job_id_t m_job_id;
            TaskExecutionMode m_exec_mode;

            ChangeListData m_data;

            void apply_change_list();

            JobSystem::job_id_t begin_task(
                TaskExecutionMode exec_mode,
                Span<typeinfo_t> read_components,
                Span<typeinfo_t> write_components);

            void end_task(JobSystem::job_id_t id);

            virtual void begin(
                IWorld* world,
                TaskExecutionMode exec_mode,
                Span<typeinfo_t> read_components,
                Span<typeinfo_t> write_components
                ) override;
            virtual void end() override;

            virtual IWorld* get_world() override { return m_world; }
            virtual R<EntityAddress> get_entity(entity_id_t id) override;
            virtual void get_clusters(Vector<Cluster*>& result, filter_func_t* filter, void* userdata) override;
            virtual entity_id_t add_entity() override { return m_data.add_entity(m_world->m_entity_id_allocator.allocate_id()); }
            virtual void remove_entity(entity_id_t id) override { return m_data.remove_entity(id); }
            virtual void remove_all_entities() override { m_data.remove_all_entities(); }
            virtual void set_target_entity(entity_id_t id) { m_data.set_target_entity(id); }
            virtual void* add_component(typeinfo_t component_type, bool allow_overwrite, usize* data_index) override { return m_data.add_component(component_type, allow_overwrite, data_index); }
            virtual void* get_temp_component_data(typeinfo_t component_type, usize index) override { return m_data.get_temp_component_data(component_type, index); }
            virtual void remove_component(typeinfo_t component_type) override { m_data.remove_component(component_type); }
            virtual void remove_all_components() override { m_data.remove_all_components(); }
            virtual void add_tag(entity_id_t tag) override { m_data.add_tag(tag); }
			virtual void remove_tag(entity_id_t tag) override { m_data.remove_tag(tag); }
			virtual void remove_all_tags() override { m_data.remove_all_tags(); }
        };
    }
} 