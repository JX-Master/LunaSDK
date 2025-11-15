/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.cpp
* @author JXMaster
* @date 2025/11/12
*/
#include "World.hpp"

namespace Luna
{
    World::World()
    {
        world = ECS::new_world();
    }
    World::~World()
    {
        actors.clear();
    }
    Actor* World::get_actor(const Guid& guid)
    {
        auto iter = actors.find(guid);
        if(iter == actors.end()) return nullptr;
        return iter->second.get();
    }
    Actor* World::add_actor(const Guid& guid, Span<const typeinfo_t> components, Span<const ECS::tag_t> tags)
    {
        auto iter = actors.find(guid);
        if(iter != actors.end()) return iter->second.get();
        UniquePtr<Actor> actor(memnew<Actor>(guid, world.get(), components, tags));
        iter = actors.insert(make_pair(guid, move(actor))).first;
        return iter->second.get();
    }
    void World::remove_actor(const Guid& guid)
    {
        auto iter = actors.find(guid);
        if(iter == actors.end()) return;
        // Remove children.
        ActorInfo* info = iter->second->get_actor_info();
        Vector<Actor*> children;
        info->get_children(children);
        for(Actor* child : children)
        {
            Guid child_guid = child->get_actor_info()->get_guid();
            remove_actor(guid);
        }
        // Remove parent.
        Actor* parent = info->get_parent();
        if(parent)
        {
            parent->remove_child(iter->second.get());
        }
        // Remove this.
        actors.erase(iter);
    }
}