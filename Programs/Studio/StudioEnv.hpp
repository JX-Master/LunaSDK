/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file StudioEnv.hpp
* @author JXMaster
* @date 2026/6/18
*/
#pragma once
#include <Luna/Asset/Asset.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Path.hpp>
#include "StudioEnv.generated.hpp"

namespace Luna
{
    namespace GUICore
    {
        struct IContext;
        struct LayoutInput;
    }

    //! @interface IAssetEditor
    //! Represents a window of the editor.
    struct [[Luna::interface("{410f7868-38b5-4e3f-b291-8e58d2cb7372}")]] IAssetEditor : virtual Interface
    {
        virtual void on_render(GUICore::IContext* context, const GUICore::LayoutInput& layout) = 0;
        virtual bool closed() = 0;
    };

    struct AssetEditorDesc
    {
        ObjRef userdata;
        //! Called when the tile is going to be drawn by a GUI Core based asset browser.
        void (*on_draw_tile_core)(GUICore::IContext* context, object_t userdata, Asset::asset_t asset, const RectF& draw_rect) = nullptr;
        //! Called while a GUI Core tile element is being built to draw a preview relative to that tile element.
        void (*on_draw_tile_preview_core)(GUICore::IContext* context, object_t userdata, Asset::asset_t asset, const RectF& relative_rect) = nullptr;
        //! Called when a new editor is requested to be open for the specified asset.
        Ref<IAssetEditor>(*new_editor)(object_t userdata, Asset::asset_t editing_asset) = nullptr;
    };

    struct AssetImporterDesc
    {
        //! Called when a new importer is requested to be open for the specified asset.
        Ref<IAssetEditor>(*new_importer)(const Path& create_dir);
    };

    struct AppEnv
    {
        HashSet<Name> new_asset_types; // Displayed on the "New" tab of asset browser.
        HashMap<Name, AssetImporterDesc> importer_types;

        HashMap<Name, AssetEditorDesc> editor_types;

        HashSet<typeinfo_t> component_types;
        HashSet<typeinfo_t> scene_component_types;

        Ref<RHI::IDevice> device;

        u32 graphics_queue;
        u32 async_compute_queue;
        u32 async_copy_queue;

        void register_asset_importer_type(const Name& name, const AssetImporterDesc& desc)
        {
            importer_types.insert(Pair<Name, AssetImporterDesc>(name, desc));
        }

        void register_asset_editor_type(const Name& name, const AssetEditorDesc& desc)
        {
            editor_types.insert(Pair<Name, AssetEditorDesc>(name, desc));
        }
    };

    extern AppEnv* g_env;
}
