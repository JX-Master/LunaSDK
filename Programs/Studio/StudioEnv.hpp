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
#include <Luna/GUI/Base.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Path.hpp>
#include "StudioEnv.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        struct IContext;
        struct LayoutConfig;
    }

    //! @interface IAssetEditor
    //! Represents a window of the editor.
    struct [[Luna::interface("{410f7868-38b5-4e3f-b291-8e58d2cb7372}")]] IAssetEditor : virtual Interface
    {
        virtual void on_render(GUI::IContext* context, const GUI::LayoutConfig& layout) = 0;
        virtual bool closed() = 0;
    };

    struct AssetEditorDesc
    {
        ObjRef userdata;
        //! Called during GUI draw-command generation to draw an asset tile.
        //! @param[in] paint_order_id The first Paint Order ID available to the callback.
        //! @return Returns the maximum Paint Order ID used by the callback, or a failure code.
        R<GUI::paint_order_id_t> (*on_draw_tile_gui)(GUI::IContext* context, object_t userdata,
            Asset::asset_t asset, const RectF& draw_rect, GUI::paint_order_id_t paint_order_id) = nullptr;
        //! Called during GUI draw-command generation to draw a preview relative to its tile element.
        //! @param[in] paint_order_id The first Paint Order ID available to the callback.
        //! @return Returns the maximum Paint Order ID used by the callback, or a failure code.
        R<GUI::paint_order_id_t> (*on_draw_tile_preview_gui)(GUI::IContext* context, object_t userdata,
            Asset::asset_t asset, const RectF& relative_rect, GUI::paint_order_id_t paint_order_id) = nullptr;
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
