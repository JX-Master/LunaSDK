/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Scene.cpp
* @author JXMaster
* @date 2025/11/12
*/
#include "Scene.hpp"
#include "World.hpp"

namespace Luna
{
    void Scene::add_to_world(World* world)
    {
        // Create actors.
        Vector<Actor*> world_actors;
        world_actors.reserve(actors.size());
        Vector<typeinfo_t> component_types;
        // Use this scene object as a tag.
        ECS::tag_t tag = this;
        for(auto& actor : actors)
        {
            component_types.clear();
            for(auto& component : actor.components)
            {
                component_types.push_back(component.type());
            }
            Actor* new_actor = world->add_actor(actor.guid, component_types.cspan(), {tag});
            ActorInfo* info = new_actor->get_actor_info();
            info->name = actor.name;
            Transform* transform = new_actor->get_transform();
            *transform = actor.transform;
            world_actors.push_back(new_actor);
        } 
        // Initialize actors.
        for(usize i = 0; i < world_actors.size(); ++i)
        {
            SceneActor& src = actors[i];
            Actor* dst = world_actors[i];
            for(auto& component : src.components)
            {
                void* data = dst->get_component(component.type());
                copy_assign_type(component.type(), data, component.get());
            }
            for(const auto& child : src.children)
            {
                Actor* child_actor = world->get_actor(child);
                dst->add_child(child_actor);
            }
        }
    }
    void Scene::remove_from_world(World* world)
    {
        for(const auto& actor : actors)
        {
            world->remove_actor(actor.guid);
        }
    }
}