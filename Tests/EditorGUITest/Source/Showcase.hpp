/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Showcase.hpp
* @author JXMaster
* @date 2026/7/17
*/
#pragma once
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/RHI/Texture.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    namespace EditorGUITest
    {
        struct ShowcaseDrawStep
        {
            GUI::DrawCommand command;
            GUI::DrawCallback callback = nullptr;
            void* userdata = nullptr;
            bool is_callback = false;
        };

        struct ShowcaseDrawData
        {
            Vector<ShowcaseDrawStep> before_children;
            Vector<ShowcaseDrawStep> after_children;
        };

        struct ShowcaseState
        {
            i32 section = 0;
            i32 theme = 0;
            i32 density = 1;
            i32 selected_asset = 1;
            i32 selected_group = 1;
            i32 space_mode = 0;
            i32 console_tab = 0;
            i32 inspector_tab = 0;
            i32 combo_item = 0;
            i32 table_selection = 0;
            i32 radio_value = 1;
            bool selected_group_multi[4] = { true, false, true, false };
            bool checkbox_value = true;
            bool receive_shadows = false;
            bool live_preview = true;
            bool normal_map = true;
            bool triplanar = true;
            bool disclosure_open = true;
            bool menu_grid = true;
            bool workspace_layout_initialized = false;
            bool pinned_inspector_open = true;
            f32 roughness = 0.36f;
            f32 metallic = 0.82f;
            i32 subdivisions = 64;
            f32 position[3] = { 1.0f, 2.0f, 3.0f };
            f32 preview_color[4] = { 0.851f, 0.325f, 0.365f, 1.0f };
            String asset_name = "M_Rusted_Metal";
            String readonly_value = "Read-only value";
            String search_query;
            String table_filter;
            Ref<RHI::ITexture> material_preview;
            Ref<RHI::ITexture> material_sand;
            Ref<RHI::ITexture> material_rusted;
            Ref<RHI::ITexture> material_concrete;
            Ref<VG::IShapeBuffer> circle_buffer;
            GUI::ShapeDesc circle;
            Vector<UniquePtr<ShowcaseDrawData>> frame_draw_data;
        };

        struct ShowcaseHandles
        {
            GUI::ElementHandle navigation[9];
            GUI::ElementHandle style_options[4];
            GUI::ElementHandle assets[3];
            GUI::ElementHandle popup_button;
        };

        GUI::ElementHandle build_showcase(GUI::IContext* context, ShowcaseState& state,
            ShowcaseHandles& handles);
        void resolve_showcase(GUI::IContext* context, ShowcaseState& state,
            const ShowcaseHandles& handles);
    }
}
