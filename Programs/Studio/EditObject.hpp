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

namespace Luna
{
    void edit_enum(const c8* name, typeinfo_t type, void* obj);

    template <typename _Ty>
    void edit_enum(const c8* name, _Ty& obj)
    {
        edit_enum(name, typeof<_Ty>(), &obj);
    }

    void edit_asset(const c8* name, Asset::asset_t& asset);

    void edit_object(object_t obj);
}