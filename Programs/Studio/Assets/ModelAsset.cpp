/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ModelAsset.cpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Model.hpp"
#include <VFS/VFS.hpp>
#include <Runtime/Serialization.hpp>
#include <Runtime/VariantJSON.hpp>
#include "../StudioHeader.hpp"

namespace Luna
{
	Name get_model_asset_type()
	{
		return "Model";
	}

	void register_model_asset_type()
	{
		register_struct_type<Model>({
			luproperty(Model, Asset::asset_t, mesh),
			luproperty(Model, Vector<Asset::asset_t>, materials)
			});
		set_serializable<Model>();
		Asset::AssetTypeDesc desc;
		desc.name = get_model_asset_type();
		desc.on_load_asset = load_json_asset<Model>;
		desc.on_save_asset = save_json_asset<Model>;
		desc.on_load_asset_default_data = create_default_object<Model>;
		desc.on_set_asset_data = nullptr;
		desc.userdata = nullptr;
		Asset::register_asset_type(desc);
	}
}