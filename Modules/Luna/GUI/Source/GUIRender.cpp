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
        static VG::TextAlignment to_vg_text_alignment(TextAlignment alignment);

        struct ContextNodeRenderContext : NodeRenderContext
        {
            Context* context = nullptr;
            IDrawList* active_draw_list = nullptr;
            id_t node_id = 0;

            virtual IDrawList* draw_list() override
            {
                return active_draw_list;
            }

            virtual const Any* get_persistent_state(const Name& key) const override
            {
                if(!context || !node_id) return nullptr;
                PersistentItemState& state = context->get_or_create_persistent_state(node_id);
                auto iter = state.custom_states.find(key);
                return iter == state.custom_states.end() ? nullptr : &iter->second;
            }

            virtual void set_persistent_state(const Name& key, const Any& value) override
            {
                if(!context || !node_id) return;
                PersistentItemState& state = context->get_or_create_persistent_state(node_id);
                state.custom_states.insert_or_assign(key, value);
            }

            virtual bool is_popup_open(id_t popup_id) const override
            {
                return context && popup_id ? context->is_popup_open(popup_id) : false;
            }

            virtual bool is_combo_open(id_t combo_id) const override
            {
                if(!context || !combo_id || context->m_open_combo_id != combo_id)
                {
                    return false;
                }
                PersistentItemState& state = context->get_or_create_persistent_state(combo_id);
                return state.open;
            }

            virtual void draw_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                RHI::ITexture* texture, ImageFlag image_flags) override
            {
                context->render_rect(rect, clip_rect, color, radius, texture, image_flags);
            }

            virtual void draw_rect_corners(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
                bool top_left, bool top_right, bool bottom_right, bool bottom_left) override
            {
                context->render_rect_corners(rect, clip_rect, color, radius, top_left, top_right, bottom_right, bottom_left);
            }

            virtual void draw_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color) override
            {
                context->render_circle(rect, clip_rect, color);
            }

            virtual void draw_line(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width) override
            {
                context->render_line_segment(begin, end, clip_rect, color, width);
            }

            virtual void draw_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color,
                TextAlignment horizontal_alignment, TextAlignment vertical_alignment) override
            {
                context->render_text(rect, clip_rect, text, font_size, color,
                    to_vg_text_alignment(horizontal_alignment), to_vg_text_alignment(vertical_alignment));
            }
        };

        static VG::TextAlignment to_vg_text_alignment(TextAlignment alignment)
        {
            switch(alignment)
            {
            case TextAlignment::center:
                return VG::TextAlignment::center;
            case TextAlignment::end:
                return VG::TextAlignment::end;
            default:
                return VG::TextAlignment::begin;
            }
        }

        static Float4U lerp_color(const Float4U& a, const Float4U& b, f32 t)
        {
            t = clamp(t, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t);
            return Float4U(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t);
        }

        static u32 color_u8(f32 value)
        {
            return (u32)color_channel_to_u8(value);
        }

        static f32 color_edit_label_width(const Node& node, const RectF& rect)
        {
            if(node.text.empty()) return 0.0f;
            return min(max((f32)node.text.size() * 8.0f + 8.0f, 80.0f), rect.width * 0.45f);
        }

        static void add_selective_rounded_rectangle(Vector<f32>& points, f32 width, f32 height, f32 radius,
            bool top_left, bool top_right, bool bottom_right, bool bottom_left)
        {
            radius = clamp(radius, 0.0f, min(width, height) * 0.5f);
            if(radius <= 0.0f || (!top_left && !top_right && !bottom_right && !bottom_left))
            {
                VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f, width, height);
                return;
            }
            VG::ShapeBuilder::move_to(points, 0.0f, bottom_left ? radius : 0.0f);
            VG::ShapeBuilder::line_to(points, 0.0f, height - (top_left ? radius : 0.0f));
            if(top_left)
            {
                VG::ShapeBuilder::circle_to(points, radius, 180.0f, 90.0f);
            }
            else
            {
                VG::ShapeBuilder::line_to(points, 0.0f, height);
            }
            VG::ShapeBuilder::line_to(points, width - (top_right ? radius : 0.0f), height);
            if(top_right)
            {
                VG::ShapeBuilder::circle_to(points, radius, 90.0f, 0.0f);
            }
            else
            {
                VG::ShapeBuilder::line_to(points, width, height);
            }
            VG::ShapeBuilder::line_to(points, width, bottom_right ? radius : 0.0f);
            if(bottom_right)
            {
                VG::ShapeBuilder::circle_to(points, radius, 0.0f, -90.0f);
            }
            else
            {
                VG::ShapeBuilder::line_to(points, width, 0.0f);
            }
            VG::ShapeBuilder::line_to(points, bottom_left ? radius : 0.0f, 0.0f);
            if(bottom_left)
            {
                VG::ShapeBuilder::circle_to(points, radius, -90.0f, -180.0f);
            }
            else
            {
                VG::ShapeBuilder::line_to(points, 0.0f, 0.0f);
            }
        }

        RectF Context::to_vg_rect(const RectF& rect) const
        {
            return RectF(rect.offset_x, m_frame_desc.surface_size.y - rect.offset_y - rect.height, rect.width, rect.height);
        }

        void Context::render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
            RHI::ITexture* texture, ImageFlag image_flags)
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
            Float2U min_texcoord(0.0f, test_flags(image_flags, ImageFlag::flip_y) ? 1.0f : 0.0f);
            Float2U max_texcoord(1.0f, test_flags(image_flags, ImageFlag::flip_y) ? 0.0f : 1.0f);
            m_active_draw_list->add_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, min_texcoord, max_texcoord);
            m_active_draw_list->pop_state(pop_id);
        }

        void Context::render_gradient_rect(const RectF& rect, const RectF& clip_rect,
            const Float4U& top_left, const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left)
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
            VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f, r.width, r.height);
            u32 end = (u32)points.size();
            VG::Vertex vertices[4];
            u32 indices[6];
            VG::get_rect_shape_draw_vertices(vertices, indices, begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                Color::white(), Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            vertices[0].color = bottom_left;
            vertices[1].color = top_left;
            vertices[2].color = top_right;
            vertices[3].color = bottom_right;
            m_active_draw_list->add_shape_raw(Span<const VG::Vertex>(vertices, 4), Span<const u32>(indices, 6));
            m_active_draw_list->pop_state(pop_id);
        }

        void Context::render_rect_corners(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius,
            bool top_left, bool top_right, bool bottom_right, bool bottom_left)
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
            add_selective_rounded_rectangle(points, r.width, r.height, radius, top_left, top_right, bottom_right, bottom_left);
            u32 end = (u32)points.size();
            m_active_draw_list->add_shape(begin, end - begin,
                Float2U(r.offset_x, r.offset_y), Float2U(r.offset_x + r.width, r.offset_y + r.height),
                Float2U(0.0f, 0.0f), Float2U(r.width, r.height),
                color, Float2U(0.0f, 0.0f), Float2U(1.0f, 1.0f));
            m_active_draw_list->pop_state(pop_id);
        }

        void Context::render_color_swatch(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius)
        {
            render_rect(rect, clip_rect, Float4U(0.24f, 0.29f, 0.36f, 1.0f), radius);
            RectF inner(rect.offset_x + 1.0f, rect.offset_y + 1.0f, max(rect.width - 2.0f, 1.0f), max(rect.height - 2.0f, 1.0f));
            f32 inner_radius = max(radius - 1.0f, 0.0f);
            f32 cell = 8.0f;
            u32 columns = max((u32)((inner.width + cell - 1.0f) / cell), 1u);
            u32 rows = max((u32)((inner.height + cell - 1.0f) / cell), 1u);
            for(u32 y = 0; y < rows; ++y)
            {
                for(u32 x = 0; x < columns; ++x)
                {
                    Float4U checker = ((x + y) & 1) ? Float4U(0.42f, 0.46f, 0.52f, 1.0f) : Float4U(0.20f, 0.23f, 0.28f, 1.0f);
                    RectF cell_rect(inner.offset_x + (f32)x * cell, inner.offset_y + (f32)y * cell,
                        min(cell, max(inner.offset_x + inner.width - (inner.offset_x + (f32)x * cell), 0.0f)),
                        min(cell, max(inner.offset_y + inner.height - (inner.offset_y + (f32)y * cell), 0.0f)));
                    render_rect(cell_rect, clip_rect, checker, 0.0f);
                }
            }
            render_rect(inner, clip_rect, color, inner_radius);
        }

        void Context::render_circle(const RectF& rect, const RectF& clip_rect, const Float4U& color)
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

        void Context::render_line_segment(const Float2U& begin, const Float2U& end, const RectF& clip_rect, const Float4U& color, f32 width)
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

        void Context::render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color, VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment)
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

        void Context::render_table_node(u32 node_index)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            const NodeLayout& layout = m_layouts[node_index];
            const RectF& rect = layout.rect;
            const RectF& clip = layout.clip_rect;
            const TableStyle& style = table_desc(node).style;
            u32 columns = layout.table_columns;
            u32 rows = layout.table_rows;
            if(!columns || !rows) return;

            u32 child = node.first_child;
            for(u32 row = 0; row < rows; ++row)
            {
                for(u32 col = 0; col < columns; ++col)
                {
                    usize cell_index = (usize)row * columns + col;
                    ColorOverride color;
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
                    else if(style.background_mode == TableBackgroundMode::solid)
                    {
                        color.enabled = true;
                        color.color = style.background_color;
                    }
                    else if(style.background_mode == TableBackgroundMode::alternate_rows)
                    {
                        color.enabled = true;
                        color.color = (row % 2) ? style.alternate_background_color : style.background_color;
                    }
                    else if(style.background_mode == TableBackgroundMode::alternate_columns)
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

        void Context::render_scrollbars(u32 node_index)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
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

        void Context::render_drag_drop_overlay()
        {
            if(!m_drag_drop_active || !m_drag_drop_type || m_layouts.size() != m_submitted_desc.nodes.size()) return;
            RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            id_t hovered_target = hit_test_drag_drop_target(m_drag_drop_type, m_pointer_pos);
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(!contains_name(node.drag_drop_target_types, m_drag_drop_type) || node.id == m_drag_drop_source_id) continue;
                const NodeLayout& layout = m_layouts[i];
                if(layout.dock_panel_child && !layout.dock_panel_visible) continue;
                RectF rect = layout.rect;
                if(rect.width <= 0.0f || rect.height <= 0.0f) continue;
                RectF clip = intersect_rect(layout.clip_rect, surface_clip);
                bool hovered = node.id == hovered_target;
                Float4U color = hovered ? Float4U(0.40f, 0.68f, 1.0f, 1.0f) : Float4U(0.28f, 0.50f, 0.86f, 0.62f);
                f32 width = hovered ? 3.0f : 2.0f;
                f32 l = rect.offset_x + 1.0f;
                f32 t = rect.offset_y + 1.0f;
                f32 r = rect.offset_x + max(rect.width - 1.0f, 1.0f);
                f32 b = rect.offset_y + max(rect.height - 1.0f, 1.0f);
                render_line_segment(Float2U(l, t), Float2U(r, t), clip, color, width);
                render_line_segment(Float2U(r, t), Float2U(r, b), clip, color, width);
                render_line_segment(Float2U(r, b), Float2U(l, b), clip, color, width);
                render_line_segment(Float2U(l, b), Float2U(l, t), clip, color, width);
            }

            if(!m_drag_drop_preview_built)
            {
                const c8* type_name = m_drag_drop_type.c_str();
                f32 width = max((f32)m_drag_drop_type.size() * 8.0f + 84.0f, 132.0f);
                RectF rect(
                    min(m_pointer_pos.x + 14.0f, max(m_frame_desc.surface_size.x - width - 8.0f, 0.0f)),
                    min(m_pointer_pos.y + 18.0f, max(m_frame_desc.surface_size.y - 34.0f, 0.0f)),
                    width,
                    30.0f);
                render_rect(rect, surface_clip, Float4U(0.08f, 0.10f, 0.13f, 0.96f), 5.0f);
                String label;
                strprintf(label, "Payload: %s", type_name);
                render_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height),
                    surface_clip, label.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
            }
        }

        void Context::render_tab_item(u32 node_index)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            const NodeLayout& layout = m_layouts[node_index];
            RectF rect = layout.tab_header_rect;
            RectF clip = layout.tab_header_clip_rect;
            if(rect.width <= 0.0f || rect.height <= 0.0f) return;

            bool hovered = node.id == m_hovered_id;
            bool active = node.id == m_active_id;
            bool selected = layout.tab_content_visible;
            bool button = test_flags(node.get_tab_item_flags(), TabItemFlag::button);
            Float4U color = selected ?
                Float4U(0.17f, 0.27f, 0.42f, 1.0f) :
                (active ? Float4U(0.17f, 0.24f, 0.34f, 1.0f) :
                    (hovered ? Float4U(0.15f, 0.20f, 0.28f, 1.0f) : Float4U(0.09f, 0.11f, 0.15f, 1.0f)));
            if(button)
            {
                color = active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                    (hovered ? Float4U(0.20f, 0.30f, 0.44f, 1.0f) : Float4U(0.12f, 0.16f, 0.22f, 1.0f));
            }
            f32 radius = 5.0f;
            render_rect(rect, clip, color, radius);
            render_rect(RectF(rect.offset_x, rect.offset_y + max(rect.height - radius, 0.0f), rect.width, min(radius, rect.height)),
                clip, color, 0.0f);
            if(selected)
            {
                render_rect(RectF(rect.offset_x, rect.offset_y, rect.width, 2.0f), clip, Float4U(0.34f, 0.60f, 0.92f, 1.0f), 1.0f);
            }

            f32 left = rect.offset_x + 9.0f;
            f32 right = rect.offset_x + rect.width - 8.0f;
            if(test_flags(node.get_tab_item_flags(), TabItemFlag::unsaved_document))
            {
                f32 dot = 6.0f;
                render_circle(RectF(left, rect.offset_y + (rect.height - dot) * 0.5f, dot, dot), clip, Float4U(0.95f, 0.64f, 0.28f, 1.0f));
                left += dot + 6.0f;
            }
            if(layout.tab_close_rect.width > 0.0f)
            {
                bool close_hovered = m_pointer_inside && point_in_rect(m_pointer_pos, layout.tab_close_rect);
                if(close_hovered)
                {
                    render_rect(layout.tab_close_rect, clip, Float4U(0.46f, 0.18f, 0.18f, 1.0f), 4.0f);
                }
                render_text(layout.tab_close_rect, clip, "X", 12.0f, Color::white(), VG::TextAlignment::center);
                right = min(right, layout.tab_close_rect.offset_x - 4.0f);
            }
            render_text(RectF(left, rect.offset_y, max(right - left, 1.0f), rect.height),
                clip, node.text.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
        }

        void Context::render_tab_scroll_buttons(u32 node_index)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            const NodeLayout& layout = m_layouts[node_index];
            if(!layout.tab_scrollable) return;
            PersistentItemState& state = get_or_create_persistent_state(node.id);
            auto render_button = [&](const RectF& rect, bool left) {
                bool enabled = left ? state.tab_scroll_x > 0.5f : state.tab_scroll_x < layout.tab_scroll_max - 0.5f;
                bool hovered = enabled && m_pointer_inside && point_in_rect(m_pointer_pos, rect);
                bool active = enabled && m_active_tab_scroll_id == node.id && m_active_tab_scroll_left == left;
                Float4U color = !enabled ? Float4U(0.09f, 0.10f, 0.12f, 0.82f) :
                    (active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                        (hovered ? Float4U(0.18f, 0.25f, 0.35f, 1.0f) : Float4U(0.11f, 0.14f, 0.19f, 0.95f)));
                render_rect(rect, layout.clip_rect, color, 4.0f);
                Float4U arrow_color = enabled ? Float4U(1.0f) : Float4U(0.45f, 0.48f, 0.52f, 1.0f);
                f32 cx = rect.offset_x + rect.width * 0.5f;
                f32 cy = rect.offset_y + rect.height * 0.5f;
                if(left)
                {
                    render_line_segment(Float2U(cx + 4.0f, cy - 6.0f), Float2U(cx - 3.0f, cy), layout.clip_rect, arrow_color, 1.8f);
                    render_line_segment(Float2U(cx - 3.0f, cy), Float2U(cx + 4.0f, cy + 6.0f), layout.clip_rect, arrow_color, 1.8f);
                }
                else
                {
                    render_line_segment(Float2U(cx - 4.0f, cy - 6.0f), Float2U(cx + 3.0f, cy), layout.clip_rect, arrow_color, 1.8f);
                    render_line_segment(Float2U(cx + 3.0f, cy), Float2U(cx - 4.0f, cy + 6.0f), layout.clip_rect, arrow_color, 1.8f);
                }
            };
            render_button(layout.tab_scroll_left_rect, true);
            render_button(layout.tab_scroll_right_rect, false);
        }

        void Context::render_dock_panel_chrome(u32 node_index)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            const NodeLayout& layout = m_layouts[node_index];
            if(!layout.dock_panel_child || !layout.dock_panel_visible) return;
            const DockPanelStyle& style = layout.dock_panel_style;
            const RectF& panel_rect = layout.dock_panel_rect;
            const RectF& clip = layout.dock_panel_clip_rect;
            bool active = node.id == m_active_dock_panel_id || node.id == m_focused_id;
            render_rect(panel_rect, clip, style.background_color, 5.0f);
            if(style.title_bar)
            {
                render_rect(layout.dock_panel_title_rect, clip, active ? style.active_title_bar_color : style.title_bar_color, 5.0f);
                const DockTreeNode* leaf = nullptr;
                auto dock_state_iter = m_persistent_states.find(layout.dock_space_id);
                if(dock_state_iter != m_persistent_states.end() && layout.dock_leaf_index < dock_state_iter->second.dock_nodes.size())
                {
                    const DockTreeNode& dock_leaf = dock_state_iter->second.dock_nodes[layout.dock_leaf_index];
                    if(!dock_leaf.split && dock_leaf.tabs.size() > 1)
                    {
                        leaf = &dock_leaf;
                    }
                }
                if(leaf)
                {
                    for(usize tab_index = 0; tab_index < leaf->tabs.size(); ++tab_index)
                    {
                        id_t tab_id = leaf->tabs[tab_index];
                        RectF tab_rect = dock_panel_tab_rect(layout.dock_panel_title_rect, tab_index, leaf->tabs.size(), style.close_button);
                        bool tab_selected = tab_id == leaf->selected_tab;
                        bool tab_hovered = m_pointer_inside && point_in_rect(m_pointer_pos, tab_rect);
                        Float4U tab_color = tab_selected ? (active ? style.active_title_bar_color : Float4U(0.16f, 0.21f, 0.28f, 1.0f)) :
                            (tab_hovered ? Float4U(0.18f, 0.24f, 0.32f, 1.0f) : Float4U(0.10f, 0.13f, 0.17f, 1.0f));
                        render_rect(tab_rect, clip, tab_color, 4.0f);
                        Node* tab_node = find_node(tab_id);
                        const c8* label = tab_node ? tab_node->text.c_str() : "";
                        render_text(RectF(tab_rect.offset_x + 7.0f, tab_rect.offset_y, max(tab_rect.width - 14.0f, 1.0f), tab_rect.height),
                            clip, label, 14.0f, Color::white(), VG::TextAlignment::begin);
                    }
                }
                else
                {
                    render_text(RectF(layout.dock_panel_title_rect.offset_x + 8.0f, layout.dock_panel_title_rect.offset_y,
                        max(layout.dock_panel_title_rect.width - 40.0f, 1.0f), layout.dock_panel_title_rect.height),
                        clip, node.text.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
                }
                if(style.close_button)
                {
                    bool close_hovered = m_pointer_inside && point_in_rect(m_pointer_pos, layout.dock_panel_close_rect);
                    render_rect(layout.dock_panel_close_rect, clip,
                        close_hovered ? Float4U(0.55f, 0.18f, 0.18f, 1.0f) : Float4U(0.23f, 0.27f, 0.33f, 1.0f),
                        4.0f);
                    render_text(layout.dock_panel_close_rect, clip, "X", 13.0f, Color::white(), VG::TextAlignment::center);
                }
            }
            if(style.border_size > 0.0f)
            {
                f32 b = style.border_size;
                render_rect(RectF(panel_rect.offset_x, panel_rect.offset_y, panel_rect.width, b), clip, style.border_color, 0.0f);
                render_rect(RectF(panel_rect.offset_x, panel_rect.offset_y + panel_rect.height - b, panel_rect.width, b), clip, style.border_color, 0.0f);
                render_rect(RectF(panel_rect.offset_x, panel_rect.offset_y, b, panel_rect.height), clip, style.border_color, 0.0f);
                render_rect(RectF(panel_rect.offset_x + panel_rect.width - b, panel_rect.offset_y, b, panel_rect.height), clip, style.border_color, 0.0f);
            }
            if(style.resize_border && layout.dock_panel_floating)
            {
                RectF r = layout.dock_panel_resize_rect;
                Float4U color = m_pointer_inside && point_in_rect(m_pointer_pos, r) ? Float4U(0.55f, 0.68f, 0.86f, 1.0f) : Float4U(0.36f, 0.42f, 0.50f, 0.85f);
                render_line_segment(Float2U(r.offset_x + r.width - 2.0f, r.offset_y + 2.0f),
                    Float2U(r.offset_x + 2.0f, r.offset_y + r.height - 2.0f), clip, color, 1.5f);
            }
            else if(style.resize_border && layout.dock_panel_resize_rect.width > 0.0f && layout.dock_panel_resize_rect.height > 0.0f)
            {
                RectF r = layout.dock_panel_resize_rect;
                Float4U color = (m_pointer_inside && point_in_rect(m_pointer_pos, r)) || node.id == m_active_dock_panel_id ?
                    Float4U(0.55f, 0.68f, 0.86f, 1.0f) :
                    Float4U(0.30f, 0.35f, 0.42f, 0.85f);
                f32 y = r.offset_y + r.height * 0.5f;
                render_line_segment(Float2U(r.offset_x + 8.0f, y), Float2U(r.offset_x + max(r.width - 8.0f, 8.0f), y), clip, color, 1.5f);
            }
        }

        void Context::render_dock_preview()
        {
            if(!m_active_dock_panel_id || !m_active_dock_panel_title_drag) return;
            DockPanelPersistentState* panel_state = find_dock_panel_state(m_active_dock_space_id, m_active_dock_panel_id);
            if(!panel_state || panel_state->mode != DockPanelMode::floating) return;

            id_t target_space_id = 0;
            u32 target_leaf = U32_MAX;
            DockDropDirection direction = DockDropDirection::none;
            if(!find_dock_drop_target(m_active_dock_panel_id, m_pointer_pos, target_space_id, target_leaf, direction)) return;
            RectF target_rect(0.0f, 0.0f, 0.0f, 0.0f);
            bool empty_dock_space = target_leaf == U32_MAX;
            if(empty_dock_space)
            {
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    if(m_submitted_desc.nodes[i].id == target_space_id && m_submitted_desc.nodes[i].is_dock_space())
                    {
                        target_rect = m_layouts[i].rect;
                        break;
                    }
                }
            }
            else
            {
                auto dock_state_iter = m_persistent_states.find(target_space_id);
                if(dock_state_iter == m_persistent_states.end() || target_leaf >= dock_state_iter->second.dock_nodes.size()) return;
                const DockTreeNode& leaf = dock_state_iter->second.dock_nodes[target_leaf];
                if(leaf.split || leaf.tabs.empty()) return;
                target_rect = leaf.rect;
            }
            if(target_rect.width <= 0.0f || target_rect.height <= 0.0f) return;

            RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            if(direction != DockDropDirection::none)
            {
                RectF preview = empty_dock_space ? target_rect : dock_drop_preview_rect(target_rect, direction);
                render_rect(preview, surface_clip, Float4U(0.20f, 0.42f, 0.78f, 0.30f), 4.0f);
                render_rect(RectF(preview.offset_x, preview.offset_y, preview.width, 2.0f), surface_clip, Float4U(0.42f, 0.68f, 1.0f, 0.95f), 0.0f);
                render_rect(RectF(preview.offset_x, preview.offset_y + max(preview.height - 2.0f, 0.0f), preview.width, 2.0f), surface_clip, Float4U(0.42f, 0.68f, 1.0f, 0.95f), 0.0f);
                render_rect(RectF(preview.offset_x, preview.offset_y, 2.0f, preview.height), surface_clip, Float4U(0.42f, 0.68f, 1.0f, 0.95f), 0.0f);
                render_rect(RectF(preview.offset_x + max(preview.width - 2.0f, 0.0f), preview.offset_y, 2.0f, preview.height), surface_clip, Float4U(0.42f, 0.68f, 1.0f, 0.95f), 0.0f);
            }

            static const DockDropDirection directions[] = {
                DockDropDirection::center,
                DockDropDirection::left,
                DockDropDirection::right,
                DockDropDirection::up,
                DockDropDirection::down
            };
            for(DockDropDirection icon_direction : directions)
            {
                if(empty_dock_space && icon_direction != DockDropDirection::center) continue;
                RectF icon = dock_drop_icon_rect(target_rect, icon_direction);
                bool selected = icon_direction == direction;
                Float4U fill = selected ? Float4U(0.27f, 0.52f, 0.88f, 0.96f) : Float4U(0.10f, 0.14f, 0.19f, 0.86f);
                Float4U stroke = selected ? Float4U(0.74f, 0.87f, 1.0f, 1.0f) : Float4U(0.46f, 0.56f, 0.68f, 0.95f);
                render_rect(icon, surface_clip, fill, 5.0f);
                f32 l = icon.offset_x + 5.0f;
                f32 r = icon.offset_x + max(icon.width - 5.0f, 5.0f);
                f32 t = icon.offset_y + 5.0f;
                f32 b = icon.offset_y + max(icon.height - 5.0f, 5.0f);
                f32 cx = icon.offset_x + icon.width * 0.5f;
                f32 cy = icon.offset_y + icon.height * 0.5f;
                if(icon_direction == DockDropDirection::center)
                {
                    render_line_segment(Float2U(l, t), Float2U(r, t), surface_clip, stroke, 1.6f);
                    render_line_segment(Float2U(r, t), Float2U(r, b), surface_clip, stroke, 1.6f);
                    render_line_segment(Float2U(r, b), Float2U(l, b), surface_clip, stroke, 1.6f);
                    render_line_segment(Float2U(l, b), Float2U(l, t), surface_clip, stroke, 1.6f);
                }
                else if(icon_direction == DockDropDirection::left || icon_direction == DockDropDirection::right)
                {
                    render_line_segment(Float2U(cx, t), Float2U(cx, b), surface_clip, stroke, 2.0f);
                }
                else
                {
                    render_line_segment(Float2U(l, cy), Float2U(r, cy), surface_clip, stroke, 2.0f);
                }
            }
        }

        void Context::render_node(u32 node_index)
        {
            const Node& node = m_submitted_desc.nodes[node_index];
            if(node.is_popup() && !popup_node_visible(node))
            {
                return;
            }
            if(node.is_tooltip() && !tooltip_node_visible(node))
            {
                return;
            }
            const RectF& rect = m_layouts[node_index].rect;
            const RectF& clip = m_layouts[node_index].clip_rect;
            IDrawList* previous_draw_list = m_active_draw_list;
            u32 dock_panel_layer_pop = U32_MAX;
            if(m_layouts[node_index].dock_panel_child && !m_layouts[node_index].dock_panel_visible)
            {
                m_active_draw_list = previous_draw_list;
                return;
            }
            if(m_layouts[node_index].dock_panel_child && m_layouts[node_index].dock_panel_floating)
            {
                DrawListState state = m_active_draw_list->get_state();
                dock_panel_layer_pop = m_active_draw_list->push_state(&state, false);
            }
            bool hovered = false;
            bool active = false;
            bool focused = false;
            auto iter = m_current_results.find(node.id);
            if(iter != m_current_results.end())
            {
                auto h = iter->second.states.find(Name("gui.hovered"));
                hovered = h != iter->second.states.end() && h->second.as<bool>() && *h->second.as<bool>();
                auto a = iter->second.states.find(Name("gui.active"));
                active = a != iter->second.states.end() && a->second.as<bool>() && *a->second.as<bool>();
                auto f = iter->second.states.find(Name("gui.focused"));
                focused = f != iter->second.states.end() && f->second.as<bool>() && *f->second.as<bool>();
            }

            if(m_layouts[node_index].dock_panel_child)
            {
                render_dock_panel_chrome(node_index);
            }

            auto render_polymorphic_node = [&]()
            {
                ContextNodeRenderContext node_render_context;
                node_render_context.context = this;
                node_render_context.active_draw_list = m_active_draw_list;
                node_render_context.node_id = node.id;
                node.render(node_render_context, rect, clip, NodeRenderState{hovered, active, focused, m_frame_desc.surface_size, m_pointer_pos, m_frame_desc.delta_time});
            };

            if(!node.uses_context_render())
            {
                render_polymorphic_node();
            }
            else if(node.is_tab_item())
            {
                render_tab_item(node_index);
            }
            else if(node.is_table_layout())
            {
                render_table_node(node_index);
            }
            else if(node.is_input_text())
            {
                render_rect(rect, clip, node.id == m_focused_id ? Float4U(0.12f, 0.16f, 0.22f, 1.0f) : Float4U(0.08f, 0.10f, 0.13f, 1.0f), 4.0f);
                String* string_value = node.string_value();
                if(string_value)
                {
                    RectF text_rect(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height);
                    RectF text_clip = intersect_rect(clip, text_rect);
                    f32 text_width = input_text_cursor_x(*string_value, string_value->size(), 16.0f);
                    RectF arrange_rect(text_rect.offset_x, text_rect.offset_y, max(text_rect.width, text_width + 4.0f), text_rect.height);
                    PersistentItemState& state = get_or_create_persistent_state(node.id);
                    state.text_cursor = clamp_utf8_cursor(*string_value, state.text_cursor);
                    if(node.id == m_focused_id && input_text_has_selection(*string_value, state))
                    {
                        usize selection_begin = 0;
                        usize selection_end = 0;
                        input_text_selection_range(*string_value, state, selection_begin, selection_end);
                        f32 selection_x0 = text_rect.offset_x + input_text_cursor_x(*string_value, selection_begin, 16.0f);
                        f32 selection_x1 = text_rect.offset_x + input_text_cursor_x(*string_value, selection_end, 16.0f);
                        render_rect(RectF(selection_x0, text_rect.offset_y + 4.0f, max(selection_x1 - selection_x0, 1.0f), max(text_rect.height - 8.0f, 1.0f)),
                            text_clip, Float4U(0.25f, 0.45f, 0.78f, 0.80f), 2.0f);
                    }
                    render_text(arrange_rect, text_clip, string_value->c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                    if(node.id == m_focused_id && !input_text_has_selection(*string_value, state))
                    {
                        f64 blink_time = max(m_time - state.text_cursor_blink_start, 0.0);
                        bool cursor_visible = (((u64)(blink_time / 0.5)) & 1) == 0;
                        if(cursor_visible)
                        {
                            f32 cursor_x = text_rect.offset_x + input_text_cursor_x(*string_value, state.text_cursor, 16.0f);
                            if(cursor_x >= text_clip.offset_x && cursor_x <= text_clip.offset_x + text_clip.width)
                            {
                                render_rect(RectF(cursor_x, text_rect.offset_y + 5.0f, 1.0f, max(text_rect.height - 10.0f, 1.0f)),
                                    text_clip, Color::white(), 0.0f);
                            }
                        }
                    }
                }
            }
            else if(node.is_color_edit())
            {
                f32 label_w = color_edit_label_width(node, rect);
                if(label_w > 0.0f)
                {
                    render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                }
                Float4U color = read_color_value(node);
                RectF value_rect(rect.offset_x + label_w, rect.offset_y + 3.0f, max(rect.width - label_w - 8.0f, 1.0f), max(rect.height - 6.0f, 1.0f));
                render_rect(value_rect, clip, active ? Float4U(0.18f, 0.29f, 0.44f, 1.0f) : (hovered ? Float4U(0.14f, 0.19f, 0.27f, 1.0f) : Float4U(0.08f, 0.10f, 0.13f, 1.0f)), 4.0f);
                f32 swatch_size = min(max(value_rect.height - 6.0f, 1.0f), 34.0f);
                RectF swatch(value_rect.offset_x + 4.0f, value_rect.offset_y + (value_rect.height - swatch_size) * 0.5f, swatch_size, swatch_size);
                render_color_swatch(swatch, clip, color, 4.0f);
                String hex;
                if(f32_value_count(node) > 3)
                {
                    strprintf(hex, "#%02X%02X%02X%02X", color_u8(color.x), color_u8(color.y), color_u8(color.z), color_u8(color.w));
                }
                else
                {
                    strprintf(hex, "#%02X%02X%02X", color_u8(color.x), color_u8(color.y), color_u8(color.z));
                }
                render_text(RectF(swatch.offset_x + swatch.width + 8.0f, value_rect.offset_y,
                    max(value_rect.offset_x + value_rect.width - swatch.offset_x - swatch.width - 14.0f, 1.0f), value_rect.height),
                    clip, hex.c_str(), 15.0f, Float4U(0.86f, 0.90f, 0.96f, 1.0f), VG::TextAlignment::begin);
            }
            else if(node.is_color_picker())
            {
                Float4U color = read_color_value(node);
                id_t owner_id = node.color_owner() ? node.color_owner() : node.id;
                PersistentItemState& state = get_or_create_persistent_state(owner_id);
                i32 axis = clamp(color_edit_axis_ref(state), 0, 5);
                f32 picker_x = 0.0f;
                f32 picker_y = 0.0f;
                f32 picker_bar = 0.0f;
                color_picker_channels_from_color(axis, color, picker_x, picker_y, picker_bar);

                RectF square = color_picker_square_rect(rect);
                RectF bar = color_picker_bar_rect(rect);
                RectF current_rect = color_picker_current_rect(rect);
                RectF original_rect = color_picker_original_rect(rect);

                if(axis == 0)
                {
                    Float4U hue_color = color_hsv_to_rgb(picker_bar, 1.0f, 1.0f, 1.0f);
                    render_gradient_rect(square, clip,
                        Color::white(), hue_color,
                        hue_color, Color::white());
                    render_gradient_rect(square, clip,
                        Float4U(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.0f, 0.0f, 0.0f, 0.0f),
                        Color::black(), Color::black());
                }
                else if(axis == 1)
                {
                    for(u32 i = 0; i < 6; ++i)
                    {
                        f32 x0 = (f32)i / 6.0f;
                        f32 x1 = (f32)(i + 1) / 6.0f;
                        RectF segment(square.offset_x + square.width * x0, square.offset_y,
                            square.width * (x1 - x0) + 0.5f, square.height);
                        Float4U left = color_from_picker_channels(axis, x0, 1.0f, picker_bar, 1.0f);
                        Float4U right = color_from_picker_channels(axis, x1, 1.0f, picker_bar, 1.0f);
                        render_gradient_rect(segment, clip, left, right, right, left);
                        render_gradient_rect(segment, clip,
                            Float4U(0.0f, 0.0f, 0.0f, 0.0f), Float4U(0.0f, 0.0f, 0.0f, 0.0f),
                            Color::black(), Color::black());
                    }
                }
                else if(axis == 2)
                {
                    Float4U gray(picker_bar, picker_bar, picker_bar, 1.0f);
                    for(u32 i = 0; i < 6; ++i)
                    {
                        f32 x0 = (f32)i / 6.0f;
                        f32 x1 = (f32)(i + 1) / 6.0f;
                        RectF segment(square.offset_x + square.width * x0, square.offset_y,
                            square.width * (x1 - x0) + 0.5f, square.height);
                        Float4U left = color_from_picker_channels(axis, x0, 1.0f, picker_bar, 1.0f);
                        Float4U right = color_from_picker_channels(axis, x1, 1.0f, picker_bar, 1.0f);
                        render_gradient_rect(segment, clip, left, right, right, left);
                        render_gradient_rect(segment, clip,
                            Float4U(gray.x, gray.y, gray.z, 0.0f), Float4U(gray.x, gray.y, gray.z, 0.0f),
                            gray, gray);
                    }
                }
                else
                {
                    render_gradient_rect(square, clip,
                        color_from_picker_channels(axis, 0.0f, 1.0f, picker_bar, 1.0f),
                        color_from_picker_channels(axis, 1.0f, 1.0f, picker_bar, 1.0f),
                        color_from_picker_channels(axis, 1.0f, 0.0f, picker_bar, 1.0f),
                        color_from_picker_channels(axis, 0.0f, 0.0f, picker_bar, 1.0f));
                }
                render_line_segment(Float2U(square.offset_x, square.offset_y), Float2U(square.offset_x + square.width, square.offset_y), clip, Float4U(0.24f, 0.29f, 0.36f, 1.0f), 1.0f);
                render_line_segment(Float2U(square.offset_x + square.width, square.offset_y), Float2U(square.offset_x + square.width, square.offset_y + square.height), clip, Float4U(0.24f, 0.29f, 0.36f, 1.0f), 1.0f);
                render_line_segment(Float2U(square.offset_x + square.width, square.offset_y + square.height), Float2U(square.offset_x, square.offset_y + square.height), clip, Float4U(0.24f, 0.29f, 0.36f, 1.0f), 1.0f);
                render_line_segment(Float2U(square.offset_x, square.offset_y + square.height), Float2U(square.offset_x, square.offset_y), clip, Float4U(0.24f, 0.29f, 0.36f, 1.0f), 1.0f);

                if(axis == 0)
                {
                    for(u32 i = 0; i < 6; ++i)
                    {
                        f32 y0 = (f32)i / 6.0f;
                        f32 y1 = (f32)(i + 1) / 6.0f;
                        RectF segment(bar.offset_x, bar.offset_y + bar.height * y0, bar.width,
                            bar.height * (y1 - y0) + 0.5f);
                        Float4U top_color = color_hsv_to_rgb(y0, 1.0f, 1.0f, 1.0f);
                        Float4U bottom_color = color_hsv_to_rgb(y1, 1.0f, 1.0f, 1.0f);
                        render_gradient_rect(segment, clip, top_color, top_color, bottom_color, bottom_color);
                    }
                }
                else
                {
                    Float4U top_color = color_from_picker_channels(axis, picker_x, picker_y, 1.0f, 1.0f);
                    Float4U bottom_color = color_from_picker_channels(axis, picker_x, picker_y, 0.0f, 1.0f);
                    render_gradient_rect(bar, clip, top_color, top_color, bottom_color, bottom_color);
                }

                f32 cursor_x = square.offset_x + picker_x * square.width;
                f32 cursor_y = square.offset_y + (1.0f - picker_y) * square.height;
                render_circle(RectF(cursor_x - 8.0f, cursor_y - 8.0f, 16.0f, 16.0f), clip, Color::white());
                render_circle(RectF(cursor_x - 5.0f, cursor_y - 5.0f, 10.0f, 10.0f), clip, color);
                f32 bar_y = bar.offset_y + (axis == 0 ? picker_bar : (1.0f - picker_bar)) * bar.height;
                render_line_segment(Float2U(bar.offset_x - 5.0f, bar_y), Float2U(bar.offset_x + bar.width + 5.0f, bar_y), clip, Color::white(), 2.0f);

                render_text(RectF(current_rect.offset_x, current_rect.offset_y - 26.0f, current_rect.width, 22.0f), clip, "Current", 15.0f, Color::white(), VG::TextAlignment::begin);
                render_color_swatch(current_rect, clip, color, 3.0f);
                render_text(RectF(original_rect.offset_x, original_rect.offset_y - 26.0f, original_rect.width, 22.0f), clip, "Original", 15.0f, Color::white(), VG::TextAlignment::begin);
                render_color_swatch(original_rect, clip, state.color_edit_original_valid ? state.color_edit_original : color, 3.0f);
            }
            else if(node.is_slider_numeric())
            {
                f32 label_w = numeric_label_width(node, rect);
                u32 value_count = numeric_value_count(node);
                for(u32 i = 0; i < value_count; ++i)
                {
                    f32* f32_values = node.f32_values();
                    i32* i32_values = node.i32_values();
                    f32 value = is_float_numeric_node(node) ? (f32_values ? f32_values[i] : 0.0f) : (i32_values ? (f32)i32_values[i] : 0.0f);
                    RectF component_rect = numeric_component_rect(node, rect, i);
                    bool active_component = active && (m_active_float_component == U32_MAX || m_active_float_component == i);
                    f32 denom = max(node.max_value() - node.min_value(), 0.0001f);
                    f32 t = clamp((value - node.min_value()) / denom, 0.0f, 1.0f);
                    f32 track_pad = min(8.0f, component_rect.width * 0.20f);
                    f32 track_x0 = component_rect.offset_x + track_pad;
                    f32 track_x1 = component_rect.offset_x + max(component_rect.width - track_pad, track_pad);
                    f32 track_y = component_rect.offset_y + component_rect.height * 0.5f;
                    Float4U track_color = hovered ? Float4U(0.12f, 0.16f, 0.22f, 1.0f) : Float4U(0.07f, 0.08f, 0.10f, 1.0f);
                    Float4U fill_color = active_component ? Float4U(0.26f, 0.43f, 0.72f, 1.0f) :
                        (hovered ? Float4U(0.22f, 0.38f, 0.64f, 1.0f) : Float4U(0.20f, 0.36f, 0.62f, 1.0f));
                    f32 knob_x = track_x0 + (track_x1 - track_x0) * t;
                    f32 track_width = active_component ? 3.0f : 2.0f;
                    render_line_segment(Float2U(track_x0, track_y), Float2U(track_x1, track_y), clip, track_color, track_width);
                    render_line_segment(Float2U(track_x0, track_y), Float2U(knob_x, track_y), clip, fill_color, track_width);
                    f32 knob_radius = active_component ? 6.5f : 5.5f;
                    render_circle(RectF(knob_x - knob_radius, track_y - knob_radius, knob_radius * 2.0f, knob_radius * 2.0f), clip, fill_color);
                }
                if(label_w > 0.0f)
                {
                    render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
                }
            }
            else if(is_numeric_node(node))
            {
                f32 label_w = numeric_label_width(node, rect);
                u32 value_count = numeric_value_count(node);
                PersistentItemState& state = get_or_create_persistent_state(node.id);
                for(u32 i = 0; i < value_count; ++i)
                {
                    f32* f32_values = node.f32_values();
                    i32* i32_values = node.i32_values();
                    f32 value = is_float_numeric_node(node) ? (f32_values ? f32_values[i] : 0.0f) : (i32_values ? (f32)i32_values[i] : 0.0f);
                    RectF component_rect = numeric_component_rect(node, rect, i);
                    bool editing_component = is_numeric_input_node(node) && node.id == m_focused_id && state.numeric_editing && state.numeric_edit_component == i;
                    Float4U bg = node.uses_f32_color_components() ? Float4U(
                        i == 0 ? value : 0.10f,
                        i == 1 ? value : 0.10f,
                        i == 2 ? value : 0.10f,
                        1.0f) : Float4U(0.12f, 0.16f, 0.22f, 1.0f);
                    bool active_component = active && (!is_numeric_node(node) || m_active_float_component == U32_MAX || m_active_float_component == i);
                    render_rect(component_rect, clip, (active_component || editing_component) ? Float4U(0.18f, 0.29f, 0.44f, 1.0f) : bg, 4.0f);
                    if(!editing_component && node.is_drag_numeric() && node.max_value() > node.min_value())
                    {
                        f32 denom = max(node.max_value() - node.min_value(), 0.0001f);
                        f32 t = clamp((value - node.min_value()) / denom, 0.0f, 1.0f);
                        render_rect(RectF(component_rect.offset_x, component_rect.offset_y + component_rect.height - 3.0f, component_rect.width * t, 3.0f), clip, Float4U(0.30f, 0.56f, 0.88f, 1.0f), 1.5f);
                    }
                    String value_text = editing_component ? state.numeric_edit_text : numeric_value_text(node, i);
                    RectF text_rect(component_rect.offset_x + 6.0f, component_rect.offset_y, max(component_rect.width - 12.0f, 1.0f), component_rect.height);
                    RectF text_clip = intersect_rect(clip, text_rect);
                    if(editing_component && input_text_has_selection(value_text, state))
                    {
                        usize selection_begin = 0;
                        usize selection_end = 0;
                        input_text_selection_range(value_text, state, selection_begin, selection_end);
                        f32 selection_x0 = text_rect.offset_x + input_text_cursor_x(value_text, selection_begin, 16.0f);
                        f32 selection_x1 = text_rect.offset_x + input_text_cursor_x(value_text, selection_end, 16.0f);
                        render_rect(RectF(selection_x0, text_rect.offset_y + 4.0f, max(selection_x1 - selection_x0, 1.0f), max(text_rect.height - 8.0f, 1.0f)),
                            text_clip, Float4U(0.25f, 0.45f, 0.78f, 0.80f), 2.0f);
                    }
                    render_text(text_rect, text_clip, value_text.c_str(), 15.0f, Color::white(), VG::TextAlignment::begin);
                    if(editing_component && !input_text_has_selection(value_text, state))
                    {
                        f64 blink_time = max(m_time - state.text_cursor_blink_start, 0.0);
                        bool cursor_visible = (((u64)(blink_time / 0.5)) & 1) == 0;
                        if(cursor_visible)
                        {
                            f32 cursor_x = text_rect.offset_x + input_text_cursor_x(value_text, state.text_cursor, 16.0f);
                            if(cursor_x >= text_clip.offset_x && cursor_x <= text_clip.offset_x + text_clip.width)
                            {
                                render_rect(RectF(cursor_x, text_rect.offset_y + 5.0f, 1.0f, max(text_rect.height - 10.0f, 1.0f)),
                                    text_clip, Color::white(), 0.0f);
                            }
                        }
                    }
                }
                render_text(RectF(rect.offset_x, rect.offset_y, label_w, rect.height), clip, node.text.c_str(), 16.0f, Color::white(), VG::TextAlignment::begin);
            }

            if(node.is_dock_space())
            {
                Vector<u32> floating_children;
                for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                {
                    if(!m_layouts[child].dock_panel_visible) continue;
                    if(m_layouts[child].dock_panel_floating)
                    {
                        floating_children.push_back(child);
                    }
                    else
                    {
                        render_node(child);
                    }
                }
                auto dock_state_iter = m_persistent_states.find(node.id);
                if(dock_state_iter != m_persistent_states.end())
                {
                    const PersistentItemState& dock_state = dock_state_iter->second;
                    if(dock_state.dock_root_node != U32_MAX && dock_state.dock_root_node < dock_state.dock_nodes.size())
                    {
                        Vector<u32> stack;
                        stack.push_back(dock_state.dock_root_node);
                        while(!stack.empty())
                        {
                            u32 dock_node_index = stack.back();
                            stack.pop_back();
                            if(dock_node_index >= dock_state.dock_nodes.size()) continue;
                            const DockTreeNode& dock_node = dock_state.dock_nodes[dock_node_index];
                            if(!dock_node.split) continue;
                            stack.push_back(dock_node.child1);
                            stack.push_back(dock_node.child0);
                            if(dock_node.split_rect.width <= 0.0f || dock_node.split_rect.height <= 0.0f) continue;
                            bool hovered_splitter = m_pointer_inside && point_in_rect(m_pointer_pos, dock_node.split_rect);
                            bool active_splitter = m_active_dock_split_space_id == node.id && m_active_dock_split_node == dock_node_index;
                            Float4U splitter_color = active_splitter ? Float4U(0.36f, 0.58f, 0.90f, 1.0f) :
                                (hovered_splitter ? Float4U(0.28f, 0.42f, 0.62f, 0.95f) : Float4U(0.11f, 0.14f, 0.18f, 0.90f));
                            render_rect(dock_node.split_rect, clip, splitter_color, 0.0f);
                        }
                    }
                }
                for(usize i = 0; i < floating_children.size(); ++i)
                {
                    for(usize j = i + 1; j < floating_children.size(); ++j)
                    {
                        if(m_layouts[floating_children[j]].dock_panel_z_order < m_layouts[floating_children[i]].dock_panel_z_order)
                        {
                            u32 tmp = floating_children[i];
                            floating_children[i] = floating_children[j];
                            floating_children[j] = tmp;
                        }
                    }
                }
                for(u32 child : floating_children)
                {
                    render_node(child);
                }
            }
            else
            {
                if(!node.is_tab_item() || m_layouts[node_index].tab_content_visible)
                {
                    for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                    {
                        render_node(child);
                    }
                }
            }
            if(node.is_scroll_view())
            {
                render_scrollbars(node_index);
            }
            if(node.is_tab_bar())
            {
                render_tab_scroll_buttons(node_index);
            }
            if(dock_panel_layer_pop != U32_MAX)
            {
                m_active_draw_list->pop_state(dock_panel_layer_pop);
            }
            m_active_draw_list = previous_draw_list;
        }

        RV Context::render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target)
        {
            lutsassert();
            if(!render_target || m_submitted_desc.nodes.empty() || m_submitted_desc.layers.empty()) return ok;
            lutry
            {
                m_shape_draw_list->reset();
                while(m_layer_draw_lists.size() < m_submitted_desc.layers.size())
                {
                    m_layer_draw_lists.push_back(new_draw_list());
                }
                m_feedback_draw_list->begin(m_shape_draw_list);
                for(usize i = 0; i < m_submitted_desc.layers.size(); ++i)
                {
                    const Layer& layer = m_submitted_desc.layers[i];
                    if(layer.root == U32_MAX || layer.root >= m_submitted_desc.nodes.size()) continue;
                    IDrawList* draw_list = m_layer_draw_lists[i].get();
                    draw_list->begin(m_shape_draw_list);
                    m_active_draw_list = draw_list;
                    render_node(layer.root);
                    m_active_draw_list = nullptr;
                    draw_list->end();
                }
                m_active_draw_list = m_feedback_draw_list.get();
                render_dock_preview();
                render_drag_drop_overlay();
                m_active_draw_list = nullptr;
                m_feedback_draw_list->end();
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
