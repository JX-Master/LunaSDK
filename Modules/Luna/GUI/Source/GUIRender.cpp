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
        RectF GUIContext::to_vg_rect(const RectF& rect) const
        {
            return RectF(rect.offset_x, m_frame_desc.surface_size.y - rect.offset_y - rect.height, rect.width, rect.height);
        }

        void GUIContext::render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius, RHI::ITexture* texture)
        {
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            DrawListState state = m_gui_draw_list->get_state();
            state.shape_buffer = m_gui_draw_list->get_shape_buffer();
            state.texture = texture;
            state.clip_rect = c;
            u32 pop_id = m_gui_draw_list->push_state(&state);
            auto& points = m_gui_draw_list->get_shape_buffer()->get_shape_points(true);
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
            m_gui_draw_list->add_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_gui_draw_list->pop_state(pop_id);
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
            DrawListState state = m_gui_draw_list->get_state();
            state.shape_buffer = m_font_atlas->get_shape_buffer();
            state.texture = nullptr;
            state.clip_rect = c;
            u32 pop_id = m_gui_draw_list->push_state(&state);
            Vector<VG::Vertex> vertices;
            Vector<u32> indices;
            VG::generate_text_arrange_result_draw_vertices(arranged, {&section, 1}, m_font_atlas, vertices, indices);
            m_gui_draw_list->add_shape_raw(vertices.cspan(), indices.cspan());
            m_gui_draw_list->pop_state(pop_id);
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

        void GUIContext::render_node(u32 node_index)
        {
            const GUINode& node = m_submitted_desc.nodes[node_index];
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
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
            case GUINodeKind::scroll_view:
                render_rect(rect, clip, Float4U(0.10f, 0.12f, 0.14f, 0.92f), 6.0f);
                break;
            case GUINodeKind::table_layout:
                render_table_node(node_index);
                break;
            case GUINodeKind::button:
                render_rect(rect, clip, active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) : (hovered ? Float4U(0.26f, 0.43f, 0.72f, 1.0f) : Float4U(0.18f, 0.28f, 0.45f, 1.0f)), 5.0f);
                render_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::center);
                break;
            case GUINodeKind::text:
                render_text(rect, clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            case GUINodeKind::checkbox:
            {
                RectF box(rect.offset_x + 2.0f, rect.offset_y + 4.0f, 18.0f, 18.0f);
                render_rect(box, clip, node.bool_value && *node.bool_value ? Float4U(0.22f, 0.55f, 0.32f, 1.0f) : Float4U(0.18f, 0.20f, 0.23f, 1.0f), 3.0f);
                RectF label(rect.offset_x + 28.0f, rect.offset_y, max(rect.width - 28.0f, 1.0f), rect.height);
                render_text(label, clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            }
            case GUINodeKind::input_text:
                render_rect(rect, clip, node.id == m_focused_id ? Float4U(0.12f, 0.16f, 0.22f, 1.0f) : Float4U(0.08f, 0.10f, 0.13f, 1.0f), 4.0f);
                if(node.string_value)
                {
                    RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                    render_text(text_rect, clip, node.string_value->c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
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
                f32 label_w = min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
                render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                RectF value_rect(rect.offset_x + label_w, rect.offset_y, max(rect.width - label_w, 1.0f), rect.height);
                render_rect(value_rect, clip, hovered ? Float4U(0.20f, 0.30f, 0.44f, 1.0f) : Float4U(0.12f, 0.16f, 0.22f, 1.0f), 4.0f);
                const c8* item_name = "";
                if(node.i32_value && *node.i32_value >= 0 && (usize)*node.i32_value < node.items.size())
                {
                    item_name = node.items[*node.i32_value].c_str();
                }
                render_text(RectF(value_rect.offset_x + 8.0f, value_rect.offset_y, max(value_rect.width - 16.0f, 1.0f), value_rect.height), clip, item_name, 16.0f, Color::white(), VG::TextAlignment::begin);
                break;
            }
            case GUINodeKind::slider_float:
            case GUINodeKind::drag_float:
            {
                f32 value = node.f32_value ? *node.f32_value : 0.0f;
                f32 label_w = min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
                render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                RectF track(rect.offset_x + label_w, rect.offset_y + 8.0f, max(rect.width - label_w - 68.0f, 1.0f), 12.0f);
                render_rect(track, clip, Float4U(0.09f, 0.11f, 0.14f, 1.0f), 6.0f);
                f32 denom = max(node.max_value - node.min_value, 0.0001f);
                f32 t = clamp((value - node.min_value) / denom, 0.0f, 1.0f);
                render_rect(RectF(track.offset_x, track.offset_y, track.width * t, track.height), clip, active ? Float4U(0.30f, 0.56f, 0.88f, 1.0f) : Float4U(0.24f, 0.43f, 0.70f, 1.0f), 6.0f);
                String value_text;
                strprintf(value_text, "%.3f", value);
                RectF value_rect(track.offset_x + track.width + 8.0f, rect.offset_y, 60.0f, rect.height);
                render_text(value_rect, clip, value_text.c_str(), 14.0f, Color::white(), VG::TextAlignment::begin);
                break;
            }
            default:
                break;
            }

            for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
            {
                render_node(child);
            }
        }

        RV GUIContext::render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target)
        {
            lutsassert();
            if(!render_target || m_submitted_desc.nodes.empty()) return ok;
            lutry
            {
                m_shape_draw_list->reset();
                m_gui_draw_list->begin(m_shape_draw_list);
                render_node(0);
                m_gui_draw_list->end();
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
