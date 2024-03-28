/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Model.hpp
* @author JXMaster
* @date 2020/5/25
*/
#pragma once
#include <Luna/Asset/Asset.hpp>

namespace Luna
{
    struct Model
    {
        lustruct("Model", "{D6D78DDD-B0A1-4B43-BFCE-7E67542FE315}");

        Asset::asset_t mesh;
        Vector<Asset::asset_t> materials;
    };
}