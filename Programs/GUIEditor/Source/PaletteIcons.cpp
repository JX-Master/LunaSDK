/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PaletteIcons.cpp
* @author JXMaster
* @date 2026/6/11
*/
#include "PaletteIcons.hpp"
#include <Luna/VG/Shapes.hpp>

namespace Luna
{
    namespace GUIEditor
    {
        static void icon_rect(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y)
        {
            VG::ShapeBuilder::add_rectangle_filled(points, min_x, min_y, max_x, max_y);
        }

        static void icon_round_rect(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius)
        {
            VG::ShapeBuilder::add_rounded_rectangle_filled(points, min_x, min_y, max_x, max_y, radius);
        }

        static void icon_line(Vector<f32>& points, f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 width)
        {
            VG::ShapeBuilder::add_line(points, p1_x, p1_y, p2_x, p2_y, width);
        }

        static void icon_circle(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius)
        {
            VG::ShapeBuilder::add_circle_filled(points, center_x, center_y, radius);
        }

        static void icon_triangle(Vector<f32>& points, const Float2U& a, const Float2U& b, const Float2U& c)
        {
            VG::ShapeBuilder::move_to(points, a.x, a.y);
            VG::ShapeBuilder::line_to(points, b.x, b.y);
            VG::ShapeBuilder::line_to(points, c.x, c.y);
        }

        static void icon_chevron_right(Vector<f32>& points, f32 x, f32 y, f32 size, f32 width)
        {
            icon_line(points, x, y, x + size, y + size, width);
            icon_line(points, x + size, y + size, x, y + size * 2.0f, width);
        }

        static GUI::ShapeDesc append_palette_icon(PaletteIcons& icons, void(*build)(Vector<f32>&))
        {
            Vector<f32>& points = icons.shape_buffer->get_shape_points(true);
            GUI::ShapeDesc desc;
            desc.buffer = icons.shape_buffer;
            desc.first_command = (u32)points.size();
            build(points);
            desc.num_commands = (u32)(points.size() - desc.first_command);
            desc.bounds = RectF(0.0f, 0.0f, 24.0f, 24.0f);
            return desc;
        }

        static void add_palette_icon(PaletteIcons& icons, const c8* type, void(*build)(Vector<f32>&))
        {
            icons.icons.insert_or_assign(Name(type), append_palette_icon(icons, build));
        }

        static void build_icon_node(Vector<f32>& points)
        {
            icon_round_rect(points, 5.0f, 5.0f, 19.0f, 19.0f, 3.0f);
            icon_circle(points, 12.0f, 12.0f, 2.0f);
        }

        static void build_icon_h_layout(Vector<f32>& points)
        {
            icon_rect(points, 4.0f, 5.0f, 8.0f, 19.0f);
            icon_rect(points, 10.0f, 5.0f, 14.0f, 19.0f);
            icon_rect(points, 16.0f, 5.0f, 20.0f, 19.0f);
        }

        static void build_icon_v_layout(Vector<f32>& points)
        {
            icon_rect(points, 5.0f, 4.0f, 19.0f, 8.0f);
            icon_rect(points, 5.0f, 10.0f, 19.0f, 14.0f);
            icon_rect(points, 5.0f, 16.0f, 19.0f, 20.0f);
        }

        static void build_icon_scroll_view(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 4.0f, 20.0f, 20.0f, 2.0f);
            icon_rect(points, 17.0f, 7.0f, 19.0f, 17.0f);
            icon_round_rect(points, 16.5f, 8.0f, 19.5f, 12.0f, 1.5f);
            icon_rect(points, 7.0f, 7.0f, 15.0f, 9.0f);
            icon_rect(points, 7.0f, 11.0f, 14.0f, 13.0f);
            icon_rect(points, 7.0f, 15.0f, 13.0f, 17.0f);
        }

        static void build_icon_grid_layout(Vector<f32>& points)
        {
            for(u32 y = 0; y < 3; ++y)
            {
                for(u32 x = 0; x < 3; ++x)
                {
                    f32 min_x = 5.0f + (f32)x * 5.0f;
                    f32 min_y = 5.0f + (f32)y * 5.0f;
                    icon_rect(points, min_x, min_y, min_x + 3.0f, min_y + 3.0f);
                }
            }
        }

        static void build_icon_canvas_layout(Vector<f32>& points)
        {
            icon_line(points, 5.0f, 12.0f, 19.0f, 12.0f, 1.5f);
            icon_line(points, 12.0f, 5.0f, 12.0f, 19.0f, 1.5f);
            icon_round_rect(points, 7.0f, 7.0f, 17.0f, 17.0f, 2.0f);
        }

        static void build_icon_table_layout(Vector<f32>& points)
        {
            icon_rect(points, 4.0f, 4.0f, 20.0f, 6.0f);
            icon_rect(points, 4.0f, 11.0f, 20.0f, 13.0f);
            icon_rect(points, 4.0f, 18.0f, 20.0f, 20.0f);
            icon_rect(points, 4.0f, 4.0f, 6.0f, 20.0f);
            icon_rect(points, 11.0f, 4.0f, 13.0f, 20.0f);
            icon_rect(points, 18.0f, 4.0f, 20.0f, 20.0f);
        }

        static void build_icon_table_row(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 9.0f, 20.0f, 15.0f, 2.0f);
            icon_rect(points, 8.0f, 9.0f, 10.0f, 15.0f);
            icon_rect(points, 14.0f, 9.0f, 16.0f, 15.0f);
        }

        static void build_icon_text(Vector<f32>& points)
        {
            icon_rect(points, 5.0f, 5.0f, 19.0f, 8.0f);
            icon_rect(points, 10.5f, 5.0f, 13.5f, 19.0f);
            icon_rect(points, 7.0f, 17.0f, 17.0f, 20.0f);
        }

        static void build_icon_button(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 7.0f, 20.0f, 17.0f, 3.0f);
            icon_rect(points, 8.0f, 11.0f, 16.0f, 13.0f);
        }

        static void build_icon_progress_bar(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 9.0f, 20.0f, 15.0f, 3.0f);
            icon_round_rect(points, 5.5f, 10.5f, 14.0f, 13.5f, 1.5f);
        }

        static void build_icon_selectable(Vector<f32>& points)
        {
            icon_round_rect(points, 5.0f, 6.0f, 19.0f, 18.0f, 2.0f);
            icon_rect(points, 8.0f, 10.0f, 16.0f, 12.0f);
            icon_rect(points, 8.0f, 14.0f, 14.0f, 16.0f);
        }

        static void build_icon_checkbox(Vector<f32>& points)
        {
            icon_round_rect(points, 5.0f, 5.0f, 19.0f, 19.0f, 2.0f);
            icon_line(points, 8.0f, 12.0f, 11.0f, 15.0f, 2.6f);
            icon_line(points, 11.0f, 15.0f, 17.0f, 8.0f, 2.6f);
        }

        static void build_icon_radio_button(Vector<f32>& points)
        {
            icon_circle(points, 12.0f, 12.0f, 7.0f);
            icon_circle(points, 12.0f, 12.0f, 3.0f);
        }

        static void build_icon_toggle_switch(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 8.0f, 20.0f, 16.0f, 4.0f);
            icon_circle(points, 16.0f, 12.0f, 3.5f);
        }

        static void build_icon_input_text(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 7.0f, 20.0f, 17.0f, 2.0f);
            icon_rect(points, 8.0f, 10.0f, 15.0f, 12.0f);
            icon_rect(points, 16.0f, 9.0f, 17.5f, 15.0f);
        }

        static void build_icon_image(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 5.0f, 20.0f, 19.0f, 2.0f);
            icon_triangle(points, Float2U(6.0f, 17.0f), Float2U(11.0f, 11.0f), Float2U(15.0f, 17.0f));
            icon_triangle(points, Float2U(11.0f, 17.0f), Float2U(16.0f, 10.0f), Float2U(20.0f, 17.0f));
            icon_circle(points, 8.0f, 8.0f, 1.5f);
        }

        static void build_icon_collapsing_header(Vector<f32>& points)
        {
            icon_chevron_right(points, 6.0f, 6.0f, 4.0f, 2.2f);
            icon_rect(points, 12.0f, 8.0f, 20.0f, 10.5f);
            icon_rect(points, 12.0f, 13.5f, 17.0f, 16.0f);
        }

        static void build_icon_tree_node(Vector<f32>& points)
        {
            icon_chevron_right(points, 5.0f, 5.0f, 4.0f, 2.0f);
            icon_line(points, 11.0f, 8.0f, 11.0f, 18.0f, 1.5f);
            icon_line(points, 11.0f, 12.0f, 18.0f, 12.0f, 1.5f);
            icon_line(points, 11.0f, 18.0f, 18.0f, 18.0f, 1.5f);
            icon_circle(points, 19.0f, 12.0f, 1.5f);
            icon_circle(points, 19.0f, 18.0f, 1.5f);
        }

        static void build_icon_button_group(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 8.0f, 20.0f, 16.0f, 3.0f);
            icon_rect(points, 9.0f, 8.0f, 10.5f, 16.0f);
            icon_rect(points, 14.0f, 8.0f, 15.5f, 16.0f);
        }

        static void build_icon_combo(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 7.0f, 20.0f, 17.0f, 2.0f);
            icon_triangle(points, Float2U(14.0f, 10.0f), Float2U(18.0f, 10.0f), Float2U(16.0f, 14.0f));
        }

        static void build_icon_slider(Vector<f32>& points)
        {
            icon_line(points, 5.0f, 12.0f, 19.0f, 12.0f, 2.0f);
            icon_circle(points, 14.0f, 12.0f, 3.0f);
        }

        static void build_icon_drag(Vector<f32>& points)
        {
            icon_line(points, 5.0f, 12.0f, 19.0f, 12.0f, 2.0f);
            icon_triangle(points, Float2U(4.0f, 12.0f), Float2U(8.0f, 8.0f), Float2U(8.0f, 16.0f));
            icon_triangle(points, Float2U(20.0f, 12.0f), Float2U(16.0f, 8.0f), Float2U(16.0f, 16.0f));
        }

        static void build_icon_asset_reference(Vector<f32>& points)
        {
            icon_round_rect(points, 4.0f, 8.0f, 12.0f, 16.0f, 3.0f);
            icon_round_rect(points, 12.0f, 8.0f, 20.0f, 16.0f, 3.0f);
            icon_line(points, 9.0f, 12.0f, 15.0f, 12.0f, 2.2f);
        }

        void init_palette_icons(PaletteIcons& icons)
        {
            icons.shape_buffer = VG::new_shape_buffer();
            icons.icons.clear();
            icons.shape_buffer->get_shape_points(true).clear();
            icons.fallback_icon = append_palette_icon(icons, build_icon_node);
            add_palette_icon(icons, "h_layout", build_icon_h_layout);
            add_palette_icon(icons, "v_layout", build_icon_v_layout);
            add_palette_icon(icons, "scroll_view", build_icon_scroll_view);
            add_palette_icon(icons, "grid_layout", build_icon_grid_layout);
            add_palette_icon(icons, "canvas_layout", build_icon_canvas_layout);
            add_palette_icon(icons, "table_layout", build_icon_table_layout);
            add_palette_icon(icons, "table_row", build_icon_table_row);
            add_palette_icon(icons, "text", build_icon_text);
            add_palette_icon(icons, "button", build_icon_button);
            add_palette_icon(icons, "progress_bar", build_icon_progress_bar);
            add_palette_icon(icons, "selectable", build_icon_selectable);
            add_palette_icon(icons, "checkbox", build_icon_checkbox);
            add_palette_icon(icons, "radio_button", build_icon_radio_button);
            add_palette_icon(icons, "toggle_switch", build_icon_toggle_switch);
            add_palette_icon(icons, "input_text", build_icon_input_text);
            add_palette_icon(icons, "image", build_icon_image);
            add_palette_icon(icons, "collapsing_header", build_icon_collapsing_header);
            add_palette_icon(icons, "tree_node", build_icon_tree_node);
            add_palette_icon(icons, "button_group", build_icon_button_group);
            add_palette_icon(icons, "combo", build_icon_combo);
            add_palette_icon(icons, "slider_float", build_icon_slider);
            add_palette_icon(icons, "slider_int", build_icon_slider);
            add_palette_icon(icons, "drag_float", build_icon_drag);
            add_palette_icon(icons, "drag_int", build_icon_drag);
            add_palette_icon(icons, "asset_reference", build_icon_asset_reference);
        }

        GUI::ShapeDesc& palette_icon(PaletteIcons& icons, const Name& type)
        {
            auto iter = icons.icons.find(type);
            return iter == icons.icons.end() ? icons.fallback_icon : iter->second;
        }
    }
}
