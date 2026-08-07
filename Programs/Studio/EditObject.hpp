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
    namespace GUICore
    {
        struct IContext;
    }

    bool edit_enum(GUICore::IContext* context, const c8* name, typeinfo_t type, void* obj);

    template <typename _Ty>
    bool edit_enum(GUICore::IContext* context, const c8* name, _Ty& obj)
    {
        return edit_enum(context, name, typeof<_Ty>(), &obj);
    }

    bool edit_asset(GUICore::IContext* context, const c8* name, Asset::asset_t& asset);

    bool edit_actor_ref(GUICore::IContext* context, const c8* name, World* world, ActorRef& ref);

    bool edit_object(GUICore::IContext* context, typeinfo_t type, void* data);

    bool edit_scene_object(GUICore::IContext* context, World* world, typeinfo_t type, void* data);
}
