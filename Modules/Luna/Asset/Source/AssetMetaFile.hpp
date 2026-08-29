/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AssetMetaFile.hpp
* @author JXMaster
* @date 2022/5/11
*/
#pragma once
#include "../Asset.hpp"
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "AssetMetaFile.generated.hpp"
namespace Luna
{
    namespace Asset
    {
        struct [[luna::struct("{93C04F6C-BC6C-4586-8CB2-7DF1B249DA21}")]] AssetMetaFile
        {
            [[Luna::property]] u32 format_version = 1;
            [[Luna::property]] Guid guid;
            [[Luna::property]] Name type;
            [[Luna::property]] Vector<Asset::AssetDataUnitDesc> data_units;
        };
        enum class AssetDataUnitOperation : u8
        {
            none = 0,
            loading = 1,
            setting = 2,
            saving = 3,
            querying_referred_assets = 4
        };
        struct AssetDataUnitEntry
        {
            Name loader;
            ObjRef data;
            AssetDataUnitOperation operation = AssetDataUnitOperation::none;
            u64 revision = 0;
        };
        // Maps to `asset_t`
        struct AssetEntry
        {
            Guid guid;
            Name type;
            Path path;
            AssetDataUnitEntry main_data_unit;
            HashMap<Name, AssetDataUnitEntry> data_units;
            bool maintenance = false;
            SpinLock lock;
        };
        void init_asset_registry();
        void close_asset_registry();
    }
}
