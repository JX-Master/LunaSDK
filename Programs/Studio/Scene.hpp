/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Scene.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Math/Quaternion.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Result.hpp>
#include "Transform.hpp"
#include "SceneSettings.hpp"
#include "Scene.generated.hpp"
namespace Luna
{
    //! The actor used to save to asset.
    struct [[luna::struct("65d3c5ba-38ea-4bf8-a40a-26c496f445ad")]] SceneActor
    {
        [[Luna::property]] Guid guid;
        [[Luna::property]] Name name;
        [[Luna::property]] Transform transform;
        [[Luna::property]] Vector<Guid> children;
        Vector<ObjRef> components;
    };

    struct World;
    struct [[luna::struct("{7402c29e-780b-4bb8-8de4-ee83a006a3e8}")]] Scene
    {
        [[Luna::property]] SceneSettings settings;
        [[Luna::property]] Vector<SceneActor> actors;
        SceneActor* get_actor(const Guid& guid)
        {
            for(auto& actor : actors)
            {
                if(actor.guid == guid)
                {
                    return &actor;
                }
            }
            return nullptr;
        }

        void add_to_world(World* world);
        void remove_from_world(World* world);
    };
}