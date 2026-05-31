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
        enum class NodeKind : u8
        {
            root,
            v_layout,
            h_layout,
            scroll_view,
            window,
            popup,
            tooltip,
            menu_bar,
            menu,
            menu_item,
            menu_separator,
            table_layout,
            grid_layout,
            canvas_layout,
            text,
            button,
            selectable,
            checkbox,
            radio_button,
            input_text,
            input_float,
            input_int,
            color_edit,
            color_preview,
            color_picker,
            image,
            collapsing_header,
            combo,
            slider_float,
            slider_int,
            drag_float,
            drag_int,
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
            tab_item,
            button_group
        };

        enum class ColorValueType : u8
        {
            f32,
            u8,
            rgba8
        };

        enum class ColorEditPart : u8
        {
            none,
            rgb,
            hsv
        };

        enum class ImageFlag : u32
        {
            none = 0x00,
            flip_y = 0x01
        };

        struct Node
        {
            id_t id = 0;
            NodeKind kind = NodeKind::root;
            u32 layer = U32_MAX;
            u32 parent = U32_MAX;
            u32 first_child = U32_MAX;
            u32 last_child = U32_MAX;
            u32 next_sibling = U32_MAX;
            u32 depth = 0;
            String text;
            String shortcut;
            Ref<RHI::ITexture> texture;
            Size requested_size;
            LayoutStyle layout_style;
            LayoutDesc layout_desc;
            bool has_dock_panel_style = false;
            DockPanelStyle dock_panel_style;
            bool* dock_panel_open = nullptr;
            TabBarFlag tab_bar_flags = TabBarFlag::none;
            TabItemFlag tab_item_flags = TabItemFlag::none;
            PopupFlag popup_flags = PopupFlag::none;
            NumericEditFlag numeric_flags = NumericEditFlag::none;
            TooltipDesc tooltip_desc;
            id_t popup_parent_id = 0;
            id_t popup_owner_id = 0;
            id_t menu_popup_id = 0;
            TableDesc table_desc;
            GridLayoutDesc grid_desc;
            CanvasLayoutDesc canvas_desc;
            bool has_canvas_item_layout = false;
            CanvasItemLayout canvas_item_layout;
            bool has_table_cell_color = false;
            Float4U table_cell_color = Float4U(0.0f);
            TreeNodeFlag tree_flags = TreeNodeFlag::none;
            u32 tree_depth = 0;
            bool absolute_position = false;
            Float2U position = Float2U(0.0f);
            bool has_user_clip_rect = false;
            RectF user_clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            bool selected = false;
            bool enabled = true;
            ImageFlag image_flags = ImageFlag::none;
            RectF paint_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            Float2U paint_line_begin = Float2U(0.0f);
            Float2U paint_line_end = Float2U(0.0f);
            Float4U paint_color = Float4U(1.0f);
            f32 paint_radius = 0.0f;
            f32 paint_line_width = 1.0f;
            f32 paint_font_size = 16.0f;
            TextAlignment paint_horizontal_alignment = TextAlignment::begin;
            TextAlignment paint_vertical_alignment = TextAlignment::center;
            bool* bool_value = nullptr;
            String* string_value = nullptr;
            i32* i32_value = nullptr;
            u8 i32_value_count = 1;
            i32 item_value = 0;
            f32* f32_value = nullptr;
            u8 f32_value_count = 1;
            u8* u8_value = nullptr;
            u32* u32_value = nullptr;
            ColorValueType color_value_type = ColorValueType::f32;
            id_t color_owner_id = 0;
            ColorEditPart color_edit_part = ColorEditPart::none;
            bool f32_color = false;
            f32 min_value = 0.0f;
            f32 max_value = 0.0f;
            f32 step_value = 0.0f;
            Vector<String> items;
            Vector<Name> drag_drop_source_types;
            Vector<Name> drag_drop_target_types;
            bool interactive = false;
        };

        struct Layer
        {
            id_t id = 0;
            u32 root = U32_MAX;
            Float2U screen_position = Float2U(0.0f);
        };

        struct Description
        {
            u64 generation = 0;
            Vector<Layer> layers;
            Vector<Node> nodes;
        };
    }
}
