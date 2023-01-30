/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2022/9/12
*/
#include <Runtime/Runtime.hpp>
#include <Runtime/Module.hpp>
#include <ECS/ECS.hpp>
#include <Runtime/Math/Vector.hpp>

#define lutest luassert_always

struct Position
{
	lustruct("Position", "{13CA006E-8EC1-4ECE-B919-188281F2EEA2}");
	Luna::Float3 position;
};

void ecs_test()
{
	using namespace Luna;
	using namespace Luna::ECS;
	register_struct_type<Position>({
		luproperty(Position, Float3, position)
		});
	{
		// Create world and task context.
		Ref<IWorld> world = new_world();
		Ref<ITaskContext> context = new_task_context();
	}
	{
		// Create/remove entity and validating.
		Ref<IWorld> world = new_world();
		Ref<ITaskContext> context = new_task_context();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		entity_id_t id = context->add_entity();
		auto r = context->get_entity(id);
		lutest(failed(r));
		lutest(r.errcode() == ECSError::entity_not_found());
		context->end();

		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		r = context->get_entity(id);
		lutest(succeeded(r));
		context->remove_entity(id);
		context->end();

		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		r = context->get_entity(id);
		lutest(failed(r));
		lutest(r.errcode() == ECSError::entity_not_found());
		context->end();
	}
	{
		// Reuse the same index will not cause former entity being relived.
		Ref<IWorld> world = new_world();
		Ref<ITaskContext> context = new_task_context();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		entity_id_t id = context->add_entity();
		context->end();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		context->remove_entity(id);
		context->end();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		entity_id_t id2 = context->add_entity();
		context->end();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		lutest(id.index == id2.index);
		lutest(id != id2);
		auto r = context->get_entity(id);
		lutest(failed(r));
		lutest(r.errcode() == ECSError::entity_not_found());
		r = context->get_entity(id2);
		lutest(succeeded(r));
		context->end();
	}
	{
		// Add, fetch and remove components.
		Ref<IWorld> world = new_world();
		Ref<ITaskContext> context = new_task_context();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		entity_id_t id = context->add_entity();
		context->set_target_entity(id);
		Position* data = context->add_component<Position>();
		data->position = Float3(30.0f, 20.0f, 100.0f);
		context->end();
		// fetch component.
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		EntityAddress addr = context->get_entity(id).get();
		data = get_cluster_components_data<Position>(addr.cluster);
		lutest(data[addr.index].position == Float3(30.0f, 20.0f, 100.0f));
		// remove component.
		context->set_target_entity(id);
		context->remove_component<Position>();
		context->end();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		addr = context->get_entity(id).get();
		data = get_cluster_components_data<Position>(addr.cluster);
		lutest(data == nullptr);
		context->end();
	}
	{
		// Add fetch and remove tags.
		Ref<IWorld> world = new_world();
		Ref<ITaskContext> context = new_task_context();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		entity_id_t id = context->add_entity();
		context->set_target_entity(id);
		auto tag = context->add_entity();
		context->add_tag(tag.value);
		context->end();
		// fetch tag.
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		EntityAddress addr = context->get_entity(id).get();
		auto tags = get_cluster_tags(addr.cluster);
		lutest(binary_search(tags.begin(), tags.end(), tag));
		// Remove tag.
		context->set_target_entity(id);
		context->remove_tag(tag.value);
		context->end();
		context->begin(world, TaskExecutionMode::exclusive, {}, {});
		addr = context->get_entity(id).get();
		auto tags2 = get_cluster_tags(addr.cluster);
		lutest(!binary_search(tags2.begin(), tags2.end(), tag));
		context->end();
	}
}

int main()
{
	Luna::init();
	lupanic_if_failed(Luna::init_modules());
	ecs_test();
	Luna::close();
	return 0;
}