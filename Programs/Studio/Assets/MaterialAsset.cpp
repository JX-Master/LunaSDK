/*!
* This file is a portion of LunaSDK.
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
        register_enum_type<MeterialType>();
        set_serializable<MeterialType>();
        register_struct_type<Material>();
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