/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Description.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Layout.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class GUINodeKind : u8
        {
            root,
            v_layout,
            h_layout,
            scroll_view,
            window,
            popup,
            menu_bar,
            menu,
            menu_item,
            menu_separator,
            table_layout,
            text,
            button,
            selectable,
            checkbox,
            input_text,
            image,
            collapsing_header,
            combo,
            slider_float,
            drag_float,
            tree_node,
            hit_box,
            draw_rect,
            draw_circle,
            draw_line,
            draw_text,
            draw_image,
            toggle_switch,
            dock_space,
            tab_bar,
            tab_item
        };

        enum class GUIRenderLayer : u8
        {
            main,
            overlay
        };

        struct GUINode
        {
            GUIID id = 0;
            GUINodeKind kind = GUINodeKind::root;
            GUIRenderLayer render_layer = GUIRenderLayer::main;
            u32 parent = U32_MAX;
            u32 first_child = U32_MAX;
            u32 last_child = U32_MAX;
            u32 next_sibling = U32_MAX;
            u32 depth = 0;
            String text;
            String shortcut;
            Ref<RHI::ITexture> texture;
            GUISize requested_size;
            GUILayoutStyle layout_style;
            GUILayoutDesc layout_desc;
            bool has_dock_panel_style = false;
            GUIDockPanelStyle dock_panel_style;
            bool* dock_panel_open = nullptr;
            GUITabBarFlag tab_bar_flags = GUITabBarFlag::none;
            GUITabItemFlag tab_item_flags = GUITabItemFlag::none;
            GUIPopupFlag popup_flags = GUIPopupFlag::none;
            GUIID popup_parent_id = 0;
            GUIID popup_owner_id = 0;
            GUIID menu_popup_id = 0;
            GUITableDesc table_desc;
            bool has_table_cell_color = false;
            Float4U table_cell_color = Float4U(0.0f);
            GUITreeNodeFlag tree_flags = GUITreeNodeFlag::none;
            u32 tree_depth = 0;
            bool absolute_position = false;
            Float2U position = Float2U(0.0f);
            bool has_user_clip_rect = false;
            RectF user_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool selected = false;
            bool enabled = true;
            RectF paint_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            Float2U paint_line_begin = Float2U(0.0f);
            Float2U paint_line_end = Float2U(0.0f);
            Float4U paint_color = Float4U(1.0f);
            f32 paint_radius = 0.0f;
            f32 paint_line_width = 1.0f;
            f32 paint_font_size = 16.0f;
            GUITextAlignment paint_horizontal_alignment = GUITextAlignment::begin;
            GUITextAlignment paint_vertical_alignment = GUITextAlignment::center;
            bool* bool_value = nullptr;
            String* string_value = nullptr;
            i32* i32_value = nullptr;
            f32* f32_value = nullptr;
            u8 f32_value_count = 1;
            bool f32_color = false;
            f32 min_value = 0.0f;
            f32 max_value = 0.0f;
            f32 step_value = 0.0f;
            Vector<String> items;
            Vector<Name> drag_drop_source_types;
            Vector<Name> drag_drop_target_types;
            bool interactive = false;
        };

        struct GUIDescription
        {
            u64 generation = 0;
            Vector<GUINode> nodes;
        };
    }
}
