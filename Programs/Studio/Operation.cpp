/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Operation.cpp
* @author JXMaster
* @date 2025/8/13
*/
#include "Operation.hpp"
#include "MainEditor.hpp"

namespace Luna
{
    void AssetEditingOp::execute()
    {
        Operation::execute();
        g_main_editor->mark_asset_as_edited(target_asset);
    }
    void AssetEditingOp::revert()
    {
        Operation::revert();
        g_main_editor->mark_asset_as_edited(target_asset);
    }
    void DiffAssetEditingOp::execute()
    {
        auto data_result = Asset::get_asset_data_unit_object(target_asset, Name());
        if(failed(data_result)) return;
        ObjRef data = data_result.get();
        if(!data) return;
        Variant data_var = serialize(data.type(), data.get()).get();
        VariantUtils::patch(data_var, delta);
        lupanic_if_failed(deserialize(data.type(), data.get(), data_var));
    }
    void DiffAssetEditingOp::revert()
    {
        auto data_result = Asset::get_asset_data_unit_object(target_asset, Name());
        if(failed(data_result)) return;
        ObjRef data = data_result.get();
        if(!data) return;
        Variant data_var = serialize(data.type(), data.get()).get();
        VariantUtils::revert(data_var, delta);
        lupanic_if_failed(deserialize(data.type(), data.get(), data_var));
    }
}
