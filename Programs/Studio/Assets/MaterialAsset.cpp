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
        Asset::AssetLoaderDesc loader_desc;
        loader_desc.name = "Studio.Material.Main";
        loader_desc.on_load_asset_data_unit = load_json_asset<Material>;
        loader_desc.on_save_asset_data_unit = save_json_asset<Material>;
        loader_desc.on_load_asset_data_unit_default_data = create_default_object<Material>;
        Asset::register_asset_loader(loader_desc);

        Asset::AssetTypeDesc type_desc;
        type_desc.name = get_material_asset_type();
        type_desc.main_data_unit_loader = loader_desc.name;
        Asset::register_asset_type(type_desc);
    }
}
