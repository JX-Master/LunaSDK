/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2022/9/12
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/ECS/ECS.hpp>
#include <Luna/ECS/World.hpp>
#include <Luna/Runtime/Math/Vector.hpp>

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
    }
    {
        // Create/remove entity and validating.
        Ref<IWorld> world = new_world();
        Cluster* empty_cluster = world->get_cluster({}, {}, true);
        lutest(empty_cluster);
        entity_id_t id = world->new_entity(empty_cluster);
        lutest(id != NULL_ENTITY);
        auto r = world->get_entity_address(id);
        lutest(succeeded(r));

        world->delete_entity(id);
        r = world->get_entity_address(id);
        lutest(failed(r));
        lutest(r.errcode() == ECSError::entity_not_found());
    }
    {
        // Reuse the same index will not cause former entity being relived.
        Ref<IWorld> world = new_world();
        Cluster* empty_cluster = world->get_cluster({}, {}, true);
        entity_id_t id = world->new_entity(empty_cluster);
        world->delete_entity(id);
        entity_id_t id2 = world->new_entity(empty_cluster);
        lutest(id != id2);
        auto r = world->get_entity_address(id);
        lutest(failed(r));
        lutest(r.errcode() == ECSError::entity_not_found());
        r = world->get_entity_address(id2);
        lutest(succeeded(r));
    }
    {
        // Add, fetch and remove components.
        Ref<IWorld> world = new_world();
        Cluster* empty_cluster = world->get_cluster({}, {}, true);
        Cluster* position_cluster = world->get_cluster({typeof<Position>()}, {}, true);

        entity_id_t id = world->new_entity(empty_cluster);
        auto r = world->get_entity_address(id);
        lutest(succeeded(r));
        lutest(get_cluster_num_entities(empty_cluster) == 1);

        // add component.
        r = world->set_entity_cluster(id, position_cluster);
        lutest(succeeded(r));
        lutest(get_cluster_num_entities(empty_cluster) == 0);
        lutest(get_cluster_num_entities(position_cluster) == 1);
        
        // fetch component and set data.
        usize chunk_id = r.get().index / CLUSTER_CHUNK_CAPACITY;
        usize index_in_chunk = r.get().index % CLUSTER_CHUNK_CAPACITY;
        Position* positions = get_cluster_components_data<Position>(position_cluster, chunk_id);
        lutest(positions);
        positions[index_in_chunk].position = Float3(30.0f, 20.0f, 100.0f);

        // remove component.
        r = world->set_entity_cluster(id, empty_cluster);
        lutest(get_cluster_num_entities(empty_cluster) == 1);
        lutest(get_cluster_num_entities(position_cluster) == 0);
    }
    {
        // Add fetch and remove tags.
        Ref<IWorld> world = new_world();
        usize tag;
        Cluster* tag_cluster = world->get_cluster({}, {&tag}, true);
       
        // add tag.
        entity_id_t id = world->new_entity(tag_cluster);
        // fetch tag.
        auto r = world->get_entity_address(id);
        lutest(succeeded(r));
        auto tags = get_cluster_tags(r.get().cluster);
        lutest(binary_search(tags.begin(), tags.end(), &tag));
        // Remove tag.
        Cluster* empty_cluster = world->get_cluster({}, {}, true);
        lutest(empty_cluster != tag_cluster);
        r = world->set_entity_cluster(id, empty_cluster);
        lutest(succeeded(r));
        tags = get_cluster_tags(r.get().cluster);
        lutest(!binary_search(tags.begin(), tags.end(), &tag));
    }
}

int main()
{
    Luna::init();
    lupanic_if_failed(Luna::add_modules({Luna::module_job_system(), Luna::module_ecs()}));
    lupanic_if_failed(Luna::init_modules());
    ecs_test();
    Luna::close();
    return 0;
}