/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file World.hpp
* @author JXMaster
* @date 2025/11/12
*/
#pragma once
#include <Luna/ECS/World.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include "Actor.hpp"

namespace Luna
{
    struct World
    {
        World();
        ~World();

        ECS::IWorld* get_ecs_world()
        {
            return world.get();
        }

        Actor* get_actor(const Guid& guid);
        Actor* add_actor(const Guid& guid, Span<const typeinfo_t> components, Span<const ECS::tag_t> tags);
        void remove_actor(const Guid& guid);

    private:
        Ref<ECS::IWorld> world;
        HashMap<Guid, UniquePtr<Actor>> actors;
    };
}
