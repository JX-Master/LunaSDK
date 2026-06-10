/*!
* This file is a portion of LunaSDK.
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
#include "Material.generated.hpp"
namespace Luna
{
    enum class [[luna::enum("{9410B062-1217-4376-AD3D-9D1D2EED8FEB}")]] MeterialType : u32
    {
        // Opaque Standard PBR: Base Color/Roughness/Normal/Metallic/Emissive
        lit [[Luna::option]] = 0,
        // Opaque Unlit(blackbody) model, emissive only. Add to the final scene buffer directly.
        unlit [[Luna::option]] = 1,
    };
    struct [[luna::struct("{a3554be6-8866-4c7e-8139-9a28708df995}")]] Material
    {
        [[Luna::property]] MeterialType material_type = MeterialType::lit;
        [[Luna::property]] Asset::asset_t base_color;
        [[Luna::property]] Asset::asset_t roughness;
        [[Luna::property]] Asset::asset_t normal;
        [[Luna::property]] Asset::asset_t metallic;
        [[Luna::property]] Asset::asset_t emissive;
        [[Luna::property]] f32 emissive_intensity = 1.0f;
    };
}