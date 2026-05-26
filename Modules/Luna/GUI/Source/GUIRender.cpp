/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIRender.cpp
* @author JXMaster
* @date 2026/5/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "GUI.hpp"
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/RHI/RHI.hpp>
#include <cstring>

namespace Luna
{
    namespace GUI
    {
        static VG::TextAlignment to_vg_text_alignment(GUITextAlignment alignment)
        {
            switch(alignment)
            {
            case GUITextAlignment::center:
                return VG::TextAlignment::center;
            case GUITextAlignment::end:
                return VG::TextAlignment::end;
            default:
                return VG::TextAlignment::begin;
            }
        }

        static Float4U lerp_color(const Float4U& a, const Float4U& b, f32 t)
        {
            return Float4U(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t);
        }

        RectF GUIContext::to_vg_rect(const RectF& rect) const
        {
            return RectF(rect.offset_x, m_frame_desc.surface_size.y - rect.offset_y - rect.height, rect.width, rect.height);
        }

        void GUIContext::render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius, RHI::ITexture* texture)
        {
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            DrawListState state = m_active_draw_list->get_state();
            state.shape_buffer = m_active_draw_list->get_shape_buffer();
            state.texture = texture;
            state.clip_rect = c;
            u32 pop_id = m_active_draw_list->push_state(&state);
            auto& points = m_active_draw_list->get_shape_buffer()->get_shape_points(true);
            u32 begin = (u32)points.size();
            if(radius > 0.0f)
            {
                VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0.0f, 0.0f, r.width, r.height, min(radius, min(r.width, r.height) * 0.5f));
            }
            else
            {
                VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f, r.width, r.height);
            }
            u32 end = (u32)points.size();
            m_active_draw_list->add_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_active_draw_list->pop_state(pop_id);
        }

        void GUIContext::render_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color)
        {
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            DrawListState state = m_active_draw_list->get_state();
            state.shape_buffer = m_active_draw_list->get_shape_buffer();
            state.texture = nullptr;
            state.clip_rect = c;
            u32 pop_id = m_active_draw_list->push_state(&state);
            auto& points = m_active_draw_list->get_shape_buffer()->get_shape_points(true);
            u32 begin = (u32)points.size();
            f32 radius = min(r.width, r.height) * 0.5f;
            VG::ShapeBuilder::add_circle_filled(points, r.width * 0.5f, r.height * 0.5f, radius);
            u32 end = (u32)points.size();
            m_active_draw_list->add_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_active_draw_list->pop_state(pop_id);
        }

        void GUIContext::render_line_segment(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width)
        {
            f32 margin = max(width, 1.0f);
            f32 dx = end.x > begin.x ? end.x - begin.x : begin.x - end.x;
            f32 dy = end.y > begin.y ? end.y - begin.y : begin.y - end.y;
            RectF bounds(
                min(begin.x, end.x) - margin,
                min(begin.y, end.y) - margin,
                max(dx + margin * 2.0f, 1.0f),
                max(dy + margin * 2.0f, 1.0f));
            RectF r = to_vg_rect(bounds);
            RectF c = to_vg_rect(clip_rect);
            DrawListState state = m_active_draw_list->get_state();
            state.shape_buffer = m_active_draw_list->get_shape_buffer();
            state.texture = nullptr;
            state.clip_rect = c;
            u32 pop_id = m_active_draw_list->push_state(&state);
            auto& points = m_active_draw_list->get_shape_buffer()->get_shape_points(true);
            u32 shape_begin = (u32)points.size();
            Float2U p1(begin.x - bounds.offset_x, bounds.height - (begin.y - bounds.offset_y));
            Float2U p2(end.x - bounds.offset_x, bounds.height - (end.y - bounds.offset_y));
            VG::ShapeBuilder::add_line(points, p1.x, p1.y, p2.x, p2.y, width);
            u32 shape_end = (u32)points.size();
            m_active_draw_list->add_shape(shape_begin, shape_end - shape_begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_active_draw_list->pop_state(pop_id);
        }

        void GUIContext::render_line(const GUINode& node, const RectF& rect, const RectF& clip_rect)
        {
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            DrawListState state = m_active_draw_list->get_state();
            state.shape_buffer = m_active_draw_list->get_shape_buffer();
            state.texture = nullptr;
            state.clip_rect = c;
            u32 pop_id = m_active_draw_list->push_state(&state);
            auto& points = m_active_draw_list->get_shape_buffer()->get_shape_points(true);
            u32 begin = (u32)points.size();
            Float2U p1(node.paint_line_begin.x - rect.offset_x, rect.height - (node.paint_line_begin.y - rect.offset_y));
            Float2U p2(node.paint_line_end.x - rect.offset_x, rect.height - (node.paint_line_end.y - rect.offset_y));
            VG::ShapeBuilder::add_line(points, p1.x, p1.y, p2.x, p2.y, node.paint_line_width);
            u32 end = (u32)points.size();
            m_active_draw_list->add_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                node.paint_color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_active_draw_list->pop_state(pop_id);
        }

        void GUIContext::render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color, VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment)
        {
            if(!text || !text[0]) return;
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            VG::TextArrangeSection section;
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            section.font_size = font_size;
            section.color = color;
            section.num_chars = strlen(text);
            auto arranged = VG::arrange_text(text, section.num_chars, {&section, 1}, r, vertical_alignment, horizontal_alignment);
            DrawListState state = m_active_draw_list->get_state();
            state.shape_buffer = m_font_atlas->get_shape_buffer();
            state.texture = nullptr;
            state.clip_rect = c;
            u32 pop_id = m_active_draw_list->push_state(&state);
            Vector<VG::Vertex> vertices;
            Vector<u32> indices;
            VG::generate_text_arrange_result_draw_vertices(arranged, {&section, 1}, m_font_atlas, vertices, indices);
            m_active_draw_list->add_shape_raw(vertices.cspan(), indices.cspan());
            m_active_draw_list->pop_state(pop_id);
        }

        void GUIContext::render_table_node(u32 node_index)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            const NodeLayout& layout = m_layouts[node_index];
            const RectF& rect = layout.rect;
            const RectF& clip = layout.clip_rect;
            const GUITableStyle& style = node.table_desc.style;
            u32 columns = layout.table_columns;
            u32 rows = layout.table_rows;
            if(!columns || !rows) return;

            u32 child = node.first_child;
            for(u32 row = 0; row < rows; ++row)
            {
                for(u32 col = 0; col < columns; ++col)
                {
                    usize cell_index = (usize)row * columns + col;
                    GUIColorOverride color;
                    if(child != U32_MAX && m_submitted_desc.nodes[child].has_table_cell_color)
                    {
                        color.enabled = true;
                        color.color = m_submitted_desc.nodes[child].table_cell_color;
                    }
                    else if(cell_index < style.cell_colors.size() && style.cell_colors[cell_index].enabled)
                    {
                        color = style.cell_colors[cell_index];
                    }
                    else if(row < style.row_colors.size() && style.row_colors[row].enabled)
                    {
                        color = style.row_colors[row];
                    }
                    else if(col < style.column_colors.size() && style.column_colors[col].enabled)
                    {
                        color = style.column_colors[col];
                    }
                    else if(style.background_mode == GUITableBackgroundMode::solid)
                    {
                        color.enabled = true;
                        color.color = style.background_color;
                    }
                    else if(style.background_mode == GUITableBackgroundMode::alternate_rows)
                    {
                        color.enabled = true;
                        color.color = (row % 2) ? style.alternate_background_color : style.background_color;
                    }
                    else if(style.background_mode == GUITableBackgroundMode::alternate_columns)
                    {
                        color.enabled = true;
                        color.color = (col % 2) ? style.alternate_background_color : style.background_color;
                    }
                    if(color.enabled)
                    {
                        render_rect(RectF(layout.table_column_offsets[col], layout.table_row_offsets[row], layout.table_column_widths[col], layout.table_row_heights[row]), clip, color.color, 0.0f);
                    }
                    if(child != U32_MAX)
                    {
                        child = m_submitted_desc.nodes[child].next_sibling;
                    }
                }
            }

            f32 table_top = layout.table_row_offsets[0];
            f32 table_bottom = layout.table_row_offsets.back() + layout.table_row_heights.back();
            f32 table_left = layout.table_column_offsets[0];
            f32 table_right = layout.table_column_offsets.back() + layout.table_column_widths.back();
            if(style.column_separators && style.separator_size > 0.0f)
            {
                for(u32 col = 0; col + 1 < columns; ++col)
                {
                    f32 x = layout.table_column_offsets[col] + layout.table_column_widths[col];
                    render_rect(RectF(x, table_top, style.separator_size, max(table_bottom - table_top, 1.0f)), clip, style.separator_color, 0.0f);
                }
            }
            if(style.row_separators && style.separator_size > 0.0f)
            {
                for(u32 row = 0; row + 1 < rows; ++row)
                {
                    f32 y = layout.table_row_offsets[row] + layout.table_row_heights[row];
                    render_rect(RectF(table_left, y, max(table_right - table_left, 1.0f), style.separator_size), clip, style.separator_color, 0.0f);
                }
            }
            if(style.border_size > 0.0f)
            {
                f32 b = style.border_size;
                render_rect(RectF(rect.offset_x, rect.offset_y, rect.width, b), clip, style.border_color, 0.0f);
                render_rect(RectF(rect.offset_x, rect.offset_y + rect.height - b, rect.width, b), clip, style.border_color, 0.0f);
                render_rect(RectF(rect.offset_x, rect.offset_y, b, rect.height), clip, style.border_color, 0.0f);
                render_rect(RectF(rect.offset_x + rect.width - b, rect.offset_y, b, rect.height), clip, style.border_color, 0.0f);
            }
        }

        void GUIContext::render_scrollbars(u32 node_index)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            const NodeLayout& layout = m_layouts[node_index];
            if(!scroll_has_vertical_bar(layout) && !scroll_has_horizontal_bar(layout)) return;

            PersistentItemState& state = get_or_create_persistent_state(node.id);
            bool hovered = false;
            if(m_pointer_inside)
            {
                if(scroll_has_vertical_bar(layout) && point_in_rect(m_pointer_pos, scroll_vertical_track_rect(layout)))
                {
                    hovered = true;
                }
                if(scroll_has_horizontal_bar(layout) && point_in_rect(m_pointer_pos, scroll_horizontal_track_rect(layout)))
                {
                    hovered = true;
                }
            }
            bool active = m_active_scrollbar_id == node.id;
            f32 target_opacity = (hovered || active) ? 0.92f : 0.35f;
            f32 blend = clamp(m_frame_desc.delta_time * 12.0f, 0.0f, 1.0f);
            state.scrollbar_opacity += (target_opacity - state.scrollbar_opacity) * blend;
            f32 alpha = clamp(state.scrollbar_opacity, 0.20f, 1.0f);
            const RectF& clip = layout.clip_rect;
            f32 radius = scroll_bar_size() * 0.5f;
            Float4U track_color(0.02f, 0.025f, 0.03f, alpha * 0.45f);
            Float4U thumb_color(0.58f, 0.68f, 0.80f, alpha);

            if(scroll_has_vertical_bar(layout))
            {
                RectF track = scroll_vertical_track_rect(layout);
                RectF thumb = scroll_vertical_thumb_rect(layout, state);
                render_rect(track, clip, track_color, radius);
                render_rect(thumb, clip, thumb_color, radius);
            }
            if(scroll_has_horizontal_bar(layout))
            {
                RectF track = scroll_horizontal_track_rect(layout);
                RectF thumb = scroll_horizontal_thumb_rect(layout, state);
                render_rect(track, clip, track_color, radius);
                render_rect(thumb, clip, thumb_color, radius);
            }
        }

        void GUIContext::render_combo_dropdown(const GUINode& node, const RectF& rect)
        {
            if(node.kind != GUINodeKind::combo || node.id != m_open_combo_id) return;
            PersistentItemState& state = get_or_create_persistent_state(node.id);
            if(!state.open) return;
            RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            RectF dropdown = combo_dropdown_rect(node, rect, m_frame_desc.surface_size);
            render_rect(dropdown, surface_clip, Float4U(0.07f, 0.09f, 0.12f, 0.98f), 5.0f);

            i32 hovered_item = -1;
            if(m_pointer_inside)
            {
                hovered_item = combo_dropdown_item_at(node, dropdown, m_pointer_pos);
            }
            i32 selected_item = node.i32_value ? *node.i32_value : -1;
            for(usize item_index = 0; item_index < node.items.size(); ++item_index)
            {
                RectF item_rect(
                    dropdown.offset_x,
                    dropdown.offset_y + combo_item_height() * (f32)item_index,
                    dropdown.width,
                    combo_item_height());
                if(item_rect.offset_y >= dropdown.offset_y + dropdown.height) break;
                bool selected = selected_item == (i32)item_index;
                bool item_hovered = hovered_item == (i32)item_index;
                if(selected || item_hovered)
                {
                    render_rect(item_rect, dropdown,
                        selected ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : Float4U(0.17f, 0.23f, 0.32f, 1.0f),
                        0.0f);
                }
                render_text(RectF(item_rect.offset_x + 8.0f, item_rect.offset_y, max(item_rect.width - 34.0f, 1.0f), item_rect.height),
                    dropdown, node.items[item_index].c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
                if(selected)
                {
                    f32 x = item_rect.offset_x + item_rect.width - 20.0f;
                    f32 y = item_rect.offset_y + item_rect.height * 0.5f;
                    render_line_segment(Float2U(x, y), Float2U(x + 4.0f, y + 4.0f), dropdown, Color::white(), 2.0f);
                    render_line_segment(Float2U(x + 4.0f, y + 4.0f), Float2U(x + 12.0f, y - 5.0f), dropdown, Color::white(), 2.0f);
                }
            }
        }

        void GUIContext::render_node(u32 node_index)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
            IDrawList* previous_draw_list = m_active_draw_list;
            m_active_draw_list = is_overlay_node(node) ? m_overlay_draw_list.get() : m_main_draw_list.get();
            bool hovered = false;
            bool active = false;
            auto iter = m_current_results.find(node.id);
            if(iter != m_current_results.end())
            {
                auto h = iter->second.states.find(Name("gui.hovered"));
                hovered = h != iter->second.states.end() && h->second.as<bool>() && *h->second.as<bool>();
                auto a = iter->second.states.find(Name("gui.active"));
                active = a != iter->second.states.end() && a->second.as<bool>() && *a->second.as<bool>();
            }

            switch(node.kind)
            {
            case GUINodeKind::window:
                render_rect(rect, clip, Float4U(0.10f, 0.12f, 0.14f, 0.92f), 6.0f);
                if(window_has_title_bar(node))
                {
                    RectF title_rect(rect.offset_x, rect.offset_y, rect.width, window_title_bar_height());
                    render_rect(title_rect, clip, Float4U(0.13f, 0.17f, 0.22f, 1.0f), 6.0f);
                    render_text(RectF(rect.offset_x + 10.0f, rect.offset_y, max(rect.width - 46.0f, 1.0f), window_title_bar_height()), clip, node.text.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
                    RectF close_rect = window_close_rect(rect);
                    bool close_hovered = m_pointer_inside && point_in_rect(m_pointer_pos, close_rect);
                    render_rect(close_rect, clip, close_hovered ? Float4U(0.55f, 0.18f, 0.18f, 1.0f) : Float4U(0.23f, 0.27f, 0.33f, 1.0f), 4.0f);
                    render_text(close_rect, clip, "X", 14.0f, Color::white(), VG::TextAlignment::center);
                }
                break;
            case GUINodeKind::scroll_view:
                render_rect(rect, clip, Float4U(0.10f, 0.12f, 0.14f, 0.92f), 6.0f);
                break;
            case GUINodeKind::popup:
                render_rect(rect, clip, Float4U(0.08f, 0.10f, 0.13f, 0.98f), 5.0f);
                break;
            case GUINodeKind::table_layout:
                render_table_node(node_index);
                break;
            case GUINodeKind::button:
                render_rect(rect, clip, active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : (hovered ? Float4U(0.26f, 0.43f, 0.72f, 1.0f) : Float4U(0.18f, 0.28f, 0.45f, 1.0f)), 5.0f);
                render_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::center);
                break;
            case GUINodeKind::selectable:
                if(node.selected || hovered || active)
                {
                    render_rect(rect, clip, active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : (hovered ? Float4U(0.20f, 0.30f, 0.44f, 1.0f) : Float4U(0.16f, 0.25f, 0.38f, 1.0f)), 4.0f);
                }
                render_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height), clip, node.text.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
                break;
            case GUINodeKind::text:
                render_text(rect, clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            case GUINodeKind::checkbox:
            {
                RectF box(rect.offset_x + 2.0f, rect.offset_y + 4.0f, 18.0f, 18.0f);
                bool checked = node.bool_value && *node.bool_value;
                render_rect(RectF(box.offset_x - 1.0f, box.offset_y - 1.0f, box.width + 2.0f, box.height + 2.0f),
                    clip, hovered ? Float4U(0.34f, 0.39f, 0.46f, 1.0f) : Float4U(0.25f, 0.29f, 0.35f, 1.0f), 4.0f);
                render_rect(box, clip, checked ? Float4U(0.22f, 0.55f, 0.32f, 1.0f) : Float4U(0.18f, 0.20f, 0.23f, 1.0f), 3.0f);
                if(checked)
                {
                    render_line_segment(
                        Float2U(box.offset_x + 4.0f, box.offset_y + 9.5f),
                        Float2U(box.offset_x + 7.5f, box.offset_y + 13.0f),
                        clip, Color::white(), 2.4f);
                    render_line_segment(
                        Float2U(box.offset_x + 7.5f, box.offset_y + 13.0f),
                        Float2U(box.offset_x + 14.5f, box.offset_y + 5.5f),
                        clip, Color::white(), 2.4f);
                }
                RectF label(rect.offset_x + 28.0f, rect.offset_y, max(rect.width - 28.0f, 1.0f), rect.height);
                render_text(label, clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            }
            case GUINodeKind::toggle_switch:
            {
                bool checked = node.bool_value && *node.bool_value;
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                if(!state.switch_animation_initialized)
                {
                    state.switch_animation = checked ? 1.0f : 0.0f;
                    state.switch_animation_initialized = true;
                }
                f32 target = checked ? 1.0f : 0.0f;
                f32 blend = clamp(m_frame_desc.delta_time * 14.0f, 0.0f, 1.0f);
                state.switch_animation += (target - state.switch_animation) * blend;
                f32 t = clamp(state.switch_animation, 0.0f, 1.0f);

                RectF track(rect.offset_x + 2.0f, rect.offset_y + 3.0f, 44.0f, 22.0f);
                Float4U off_track = hovered ? Float4U(0.18f, 0.20f, 0.23f, 1.0f) : Float4U(0.12f, 0.14f, 0.16f, 1.0f);
                Float4U on_track = hovered ? Float4U(0.25f, 0.62f, 0.38f, 1.0f) : Float4U(0.20f, 0.55f, 0.32f, 1.0f);
                render_rect(track, clip, lerp_color(off_track, on_track, t), track.height * 0.5f);

                f32 knob_size = 18.0f;
                f32 knob_x = track.offset_x + 2.0f + (track.width - knob_size - 4.0f) * t;
                RectF knob(knob_x, track.offset_y + 2.0f, knob_size, knob_size);
                render_circle(knob, clip, lerp_color(Float4U(0.78f, 0.80f, 0.84f, 1.0f), Color::white(), t));

                RectF label(rect.offset_x + 56.0f, rect.offset_y, max(rect.width - 56.0f, 1.0f), rect.height);
                render_text(label, clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            }
            case GUINodeKind::input_text:
                render_rect(rect, clip, node.id == m_focused_id ? Float4U(0.12f, 0.16f, 0.22f, 1.0f) : Float4U(0.08f, 0.10f, 0.13f, 1.0f), 4.0f);
                if(node.string_value)
                {
                    RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                    RectF text_clip = intersect_rect(clip, text_rect);
                    f32 text_width = input_text_cursor_x(*node.string_value, node.string_value->size(), 16.0f);
                    RectF arrange_rect(text_rect.offset_x, text_rect.offset_y, max(text_rect.width, text_width + 4.0f), text_rect.height);
                    PersistentItemState& state = get_or_create_persistent_state(node.id);
                    state.text_cursor = clamp_utf8_cursor(*node.string_value, state.text_cursor);
                    if(node.id == m_focused_id && input_text_has_selection(*node.string_value, state))
                    {
                        usize selection_begin = 0;
                        usize selection_end = 0;
                        input_text_selection_range(*node.string_value, state, selection_begin, selection_end);
                        f32 selection_x0 = text_rect.offset_x + input_text_cursor_x(*node.string_value, selection_begin, 16.0f);
                        f32 selection_x1 = text_rect.offset_x + input_text_cursor_x(*node.string_value, selection_end, 16.0f);
                        render_rect(RectF(selection_x0, text_rect.offset_y + 4.0f, max(selection_x1 - selection_x0, 1.0f), max(text_rect.height - 8.0f, 1.0f)),
                            text_clip, Float4U(0.25f, 0.45f, 0.78f, 0.80f), 2.0f);
                    }
                    render_text(arrange_rect, text_clip, node.string_value->c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                    if(node.id == m_focused_id && !input_text_has_selection(*node.string_value, state))
                    {
                        f64 blink_time = max(m_time - state.text_cursor_blink_start, 0.0);
                        bool cursor_visible = (((u64)(blink_time / 0.5)) & 1) == 0;
                        if(cursor_visible)
                        {
                            f32 cursor_x = text_rect.offset_x + input_text_cursor_x(*node.string_value, state.text_cursor, 16.0f);
                            if(cursor_x >= text_clip.offset_x && cursor_x <= text_clip.offset_x + text_clip.width)
                            {
                                render_rect(RectF(cursor_x, text_rect.offset_y + 5.0f, 1.0f, max(text_rect.height - 10.0f, 1.0f)),
                                    text_clip, Color::white(), 0.0f);
                            }
                        }
                    }
                }
                break;
            case GUINodeKind::image:
                render_rect(rect, clip, Color::white(), 0.0f, node.texture);
                break;
            case GUINodeKind::collapsing_header:
                render_rect(rect, clip, hovered ? Float4U(0.22f, 0.27f, 0.34f, 1.0f) : Float4U(0.16f, 0.19f, 0.24f, 1.0f), 4.0f);
                render_text(RectF(rect.offset_x + 8.0f, rect.offset_y, rect.width - 8.0f, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            case GUINodeKind::combo:
            {
                f32 label_w = combo_label_width(node, rect);
                render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                RectF value_rect = combo_value_rect(node, rect);
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                bool open = state.open && node.id == m_open_combo_id;
                render_rect(value_rect, clip, open ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : (hovered ? Float4U(0.20f, 0.30f, 0.44f, 1.0f) : Float4U(0.12f, 0.16f, 0.22f, 1.0f)), 4.0f);
                const c8* item_name = "";
                if(node.i32_value && *node.i32_value >= 0 && (usize)*node.i32_value < node.items.size())
                {
                    item_name = node.items[*node.i32_value].c_str();
                }
                render_text(RectF(value_rect.offset_x + 8.0f, value_rect.offset_y, max(value_rect.width - 34.0f, 1.0f), value_rect.height), clip, item_name, 16.0f, Color::white(), VG::TextAlignment::begin);
                f32 arrow_x = value_rect.offset_x + value_rect.width - 18.0f;
                f32 arrow_y = value_rect.offset_y + value_rect.height * 0.5f;
                if(open)
                {
                    render_line_segment(Float2U(arrow_x - 5.0f, arrow_y + 3.0f), Float2U(arrow_x, arrow_y - 3.0f), clip, Color::white(), 1.8f);
                    render_line_segment(Float2U(arrow_x, arrow_y - 3.0f), Float2U(arrow_x + 5.0f, arrow_y + 3.0f), clip, Color::white(), 1.8f);
                }
                else
                {
                    render_line_segment(Float2U(arrow_x - 5.0f, arrow_y - 3.0f), Float2U(arrow_x, arrow_y + 3.0f), clip, Color::white(), 1.8f);
                    render_line_segment(Float2U(arrow_x, arrow_y + 3.0f), Float2U(arrow_x + 5.0f, arrow_y - 3.0f), clip, Color::white(), 1.8f);
                }
                if(open)
                {
                    IDrawList* combo_draw_list = m_active_draw_list;
                    m_active_draw_list = m_overlay_draw_list.get();
                    render_combo_dropdown(node, rect);
                    m_active_draw_list = combo_draw_list;
                }
                break;
            }
            case GUINodeKind::slider_float:
            case GUINodeKind::drag_float:
            {
                f32 label_w = min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
                u32 value_count = node.kind == GUINodeKind::slider_float ? 1 : f32_value_count(node);
                f32 gap = 4.0f;
                f32 value_area_x = rect.offset_x + label_w;
                f32 value_area_w = max(rect.width - label_w - 8.0f, 1.0f);
                f32 component_w = max((value_area_w - gap * (f32)(value_count - 1)) / (f32)value_count, 1.0f);
                for(u32 i = 0; i < value_count; ++i)
                {
                    f32 value = node.f32_value ? node.f32_value[i] : 0.0f;
                    RectF component_rect(value_area_x + (component_w + gap) * (f32)i, rect.offset_y + 3.0f, component_w, max(rect.height - 6.0f, 1.0f));
                    Float4U bg = node.f32_color ? Float4U(
                        i == 0 ? value : 0.10f,
                        i == 1 ? value : 0.10f,
                        i == 2 ? value : 0.10f,
                        1.0f) : Float4U(0.12f, 0.16f, 0.22f, 1.0f);
                    render_rect(component_rect, clip, active ? Float4U(0.18f, 0.29f, 0.44f, 1.0f) : bg, 4.0f);
                    if(node.max_value > node.min_value)
                    {
                        f32 denom = max(node.max_value - node.min_value, 0.0001f);
                        f32 t = clamp((value - node.min_value) / denom, 0.0f, 1.0f);
                        render_rect(RectF(component_rect.offset_x, component_rect.offset_y + component_rect.height - 3.0f, component_rect.width * t, 3.0f), clip, Float4U(0.30f, 0.56f, 0.88f, 1.0f), 1.5f);
                    }
                    String value_text;
                    strprintf(value_text, "%.3f", value);
                    render_text(RectF(component_rect.offset_x + 6.0f, component_rect.offset_y, max(component_rect.width - 12.0f, 1.0f), component_rect.height), clip, value_text.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
                }
                render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            }
            case GUINodeKind::draw_rect:
                render_rect(rect, clip, node.paint_color, node.paint_radius);
                break;
            case GUINodeKind::draw_circle:
                render_circle(rect, clip, node.paint_color);
                break;
            case GUINodeKind::draw_line:
                render_line(node, rect, clip);
                break;
            case GUINodeKind::draw_text:
                render_text(rect, clip, node.text.c_str(), node.paint_font_size, node.paint_color,
                    to_vg_text_alignment(node.paint_horizontal_alignment),
                    to_vg_text_alignment(node.paint_vertical_alignment));
                break;
            case GUINodeKind::draw_image:
                render_rect(rect, clip, node.paint_color, 0.0f, node.texture);
                break;
            case GUINodeKind::hit_box:
                break;
            default:
                break;
            }

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                render_node(child);
            }
            if(node.kind == GUINodeKind::scroll_view)
            {
                render_scrollbars(node_index);
            }
            m_active_draw_list = previous_draw_list;
        }

        RV GUIContext::render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target)
        {
            lutsassert();
            if(!render_target || m_submitted_desc.nodes.empty()) return ok;
            lutry
            {
                m_shape_draw_list->reset();
                m_main_draw_list->begin(m_shape_draw_list);
                m_overlay_draw_list->begin(m_shape_draw_list);
                m_active_draw_list = m_main_draw_list.get();
                render_node(0);
                m_active_draw_list = nullptr;
                m_main_draw_list->end();
                m_overlay_draw_list->end();
                luexp(m_shape_draw_list->compile());
                luexp(m_shape_renderer->begin(render_target));
                Float4x4 mat = ProjectionMatrix::make_orthographic_off_center(0.0f, m_frame_desc.surface_size.x, 0.0f, m_frame_desc.surface_size.y, 0.0f, 1.0f);
                Float4x4U umat(mat);
                m_shape_renderer->draw(m_shape_draw_list->get_vertex_buffer(), m_shape_draw_list->get_index_buffer(), m_shape_draw_list->get_draw_calls(), &umat);
                luexp(m_shape_renderer->end());
                m_shape_renderer->submit(cmdbuf);
            }
            lucatchret;
            return ok;
        }
    }
}
