/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EditObject.hpp
* @author JXMaster
* @date 2020/5/29
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Asset/Asset.hpp>
#include "World.hpp"

namespace Luna
{
    bool edit_enum(const c8* name, typeinfo_t type, void* obj);

    template <typename _Ty>
    bool edit_enum(const c8* name, _Ty& obj)
    {
        return edit_enum(name, typeof<_Ty>(), &obj);
    }

    bool edit_asset(const c8* name, Asset::asset_t& asset);

    bool edit_actor_ref(const c8* name, World* world, ActorRef& ref);

    bool edit_object(typeinfo_t type, void* data);

    bool edit_scene_object(World* world, typeinfo_t type, void* data);
}