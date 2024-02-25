/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file IMaterial.hpp
* @author JXMaster
* @date 2020/5/25
*/
#pragma once
#include <Luna/RHI/RHI.hpp>
#include <Luna/Asset/Asset.hpp>

namespace Luna
{
    enum class MeterialType : u32
    {
        // Opaque Standard PBR: Base Color/Roughness/Normal/Metallic/Emissive
        lit = 0,
        // Opaque Unlit(blackbody) model, emissive only. Add to the final scene buffer directly.
        unlit = 1,
    };

    luenum(MeterialType, "MaterialType", "{9410B062-1217-4376-AD3D-9D1D2EED8FEB}");

    struct Material
    {
        lustruct("Material", "{a3554be6-8866-4c7e-8139-9a28708df995}");

        MeterialType material_type = MeterialType::lit;
        Asset::asset_t base_color;
        Asset::asset_t roughness;
        Asset::asset_t normal;
        Asset::asset_t metallic;
        Asset::asset_t emissive;
    };
}