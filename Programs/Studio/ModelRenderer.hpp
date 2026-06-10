/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ModelRenderer.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Asset/Asset.hpp>
#include "ModelRenderer.generated.hpp"
namespace Luna
{
    struct [[luna::struct("{27C69426-9BFB-4558-9904-9C5A05727E8C}")]] ModelRenderer
    {
        [[Luna::property]] Asset::asset_t model;
    };
}