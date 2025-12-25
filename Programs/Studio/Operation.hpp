/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Operation.hpp
* @author JXMaster
* @date 2025/8/13
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/VariantUtils/Diff.hpp>
#include <Luna/Runtime/Serialization.hpp>

namespace Luna
{
    //! The base class for all operations that can be undo/redo.
    struct Operation
    {
        lustruct("GBA::Operation", "e3902eda-6abd-4c1a-9934-1baee0419c7a");

        virtual void execute() {};
        virtual void revert() {};
    };

    //! The operation that edits one asset.
    struct AssetEditingOp : Operation
    {
        lustruct("GBA::AssetEditingOp", "9bfd334d-7134-4e70-a618-f6315cb5d5ee");

        Asset::asset_t target_asset;

        virtual void execute() override;
        virtual void revert() override;
    };

    //! A generic asset editing op that serializes differences between two asset versions.
    struct DiffAssetEditingOp : AssetEditingOp
    {
        lustruct("GBA::DiffAssetEditingOp", "9c72d43b-a531-4dc3-9e84-f79860b8005f");

        Variant delta;

        DiffAssetEditingOp() = default;

        template <typename _Ty>
        void set_data(const _Ty& before, const _Ty& after)
        {
            Variant before_var = serialize<_Ty>(before).get();
            Variant after_var = serialize<_Ty>(after).get();
            delta = VariantUtils::diff(before_var, after_var);
        }

        void set_data_variant(const Variant& before, const Variant& after)
        {
            delta = VariantUtils::diff(before, after);
        }

        virtual void execute() override;
        virtual void revert() override;
    };
}