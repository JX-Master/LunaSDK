/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MaterialAsset.cpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Material.hpp"
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include "../StudioHeader.hpp"

namespace Luna
{
    Name get_material_asset_type()
    {
        return "Material";
    }

    void register_material_asset_type()
    {
        register_enum_type<MeterialType>({
            luoption(MeterialType, lit),
            luoption(MeterialType, unlit)
            });
        set_serializable<MeterialType>();
        register_struct_type<Material>({
            luproperty(Material, MeterialType, material_type),
            luproperty(Material, Asset::asset_t, base_color),
            luproperty(Material, Asset::asset_t, roughness),
            luproperty(Material, Asset::asset_t, normal),
            luproperty(Material, Asset::asset_t, metallic),
            luproperty(Material, Asset::asset_t, emissive),
            luproperty(Material, f32, emissive_intensity)
            });
        set_serializable<Material>();
        Asset::AssetTypeDesc desc;
        desc.name = get_material_asset_type();
        desc.on_load_asset = load_json_asset<Material>;
        desc.on_save_asset = save_json_asset<Material>;
        desc.on_load_asset_default_data = create_default_object<Material>;
        desc.on_set_asset_data = nullptr;
        desc.userdata = nullptr;
        Asset::register_asset_type(desc);
    }
}