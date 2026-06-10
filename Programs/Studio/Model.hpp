/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Model.hpp
* @author JXMaster
* @date 2020/5/25
*/
#pragma once
#include <Luna/Asset/Asset.hpp>
#include "Model.generated.hpp"
namespace Luna
{
    struct [[luna::struct("{D6D78DDD-B0A1-4B43-BFCE-7E67542FE315}")]] Model
    {
        [[Luna::property]] Asset::asset_t mesh;
        [[Luna::property]] Vector<Asset::asset_t> materials;
    };
}