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
#include "RenderProxies/DockRenderProxies.hpp"
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
            u32 node_index = U32_MAX;
            id_t node_id = 0;

            virtual u32 current_node_index() const override
            {
                return node_index;
            }

            virtual const Node* get_node(u32 index) const override
            {
                if(!context || index >= context->m_submitted_desc.nodes.size()) return nullptr;
                return &context->m_submitted_desc.nodes[index];
            }

            virtual const Node* find_node(id_t id) const override
            {
                return context ? context->find_node(id) : nullptr;
            }

            virtual bool get_node_render_layout(u32 index, NodeRenderLayout& out_layout) const override
            {
                if(!context || index >= context->m_layouts.size()) return false;
                const NodeLayout& layout = context->m_layouts[index];
                out_layout.rect = layout.rect;
                out_layout.clip_rect = layout.clip_rect;
                out_layout.table_column_offsets = layout.table_column_offsets;
                out_layout.table_column_widths = layout.table_column_widths;
                out_layout.table_row_offsets = layout.table_row_offsets;
                out_layout.table_row_heights = layout.table_row_heights;
                out_layout.table_columns = layout.table_columns;
                out_layout.table_rows = layout.table_rows;
                out_layout.tab_header_rect = layout.tab_header_rect;
                out_layout.tab_header_clip_rect = layout.tab_header_clip_rect;
                out_layout.tab_close_rect = layout.tab_close_rect;
                out_layout.tab_scroll_left_rect = layout.tab_scroll_left_rect;
                out_layout.tab_scroll_right_rect = layout.tab_scroll_right_rect;
                out_layout.tab_scrollable = layout.tab_scrollable;
                out_layout.tab_scroll_max = layout.tab_scroll_max;
                out_layout.tab_content_visible = layout.tab_content_visible;
                out_layout.dock_panel_child = layout.dock_panel_child;
                out_layout.dock_panel_visible = layout.dock_panel_visible;
                out_layout.dock_panel_floating = layout.dock_panel_floating;
                out_layout.dock_space_id = layout.dock_space_id;
                out_layout.dock_panel_rect = layout.dock_panel_rect;
                out_layout.dock_panel_clip_rect = layout.dock_panel_clip_rect;
                out_layout.dock_panel_title_rect = layout.dock_panel_title_rect;
                out_layout.dock_panel_close_rect = layout.dock_panel_close_rect;
                out_layout.dock_panel_resize_rect = layout.dock_panel_resize_rect;
                out_layout.dock_panel_style = layout.dock_panel_style;
                out_layout.dock_leaf_index = layout.dock_leaf_index;
                out_layout.scroll_content_size = layout.scroll_content_size;
                out_layout.scroll_viewport_size = layout.scroll_viewport_size;
                out_layout.scroll_has_vertical = layout.scroll_has_vertical;
                out_layout.scroll_has_horizontal = layout.scroll_has_horizontal;
                return true;
            }

            virtual IDrawList* draw_list() override
            {
                return active_draw_list;
            }

            virtual object_t get_state(id_t id) const override
            {
                return context ? context->get_state_object(id) : nullptr;
            }

            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) override
            {
                return context ? context->set_state(id, data, lifetime) : BasicError::bad_arguments();
            }

            virtual void clear_state(id_t id) override
            {
                if(context)
                {
                    context->clear_state(id);
                }
            }

            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) const override
            {
                return context ? context->get_style_value(style, entry, default_value) : default_value;
            }

            virtual f32 text_cursor_x(const String& text, usize cursor, f32 font_size) const override
            {
                if(!context) return 0.0f;
                const Node* node = get_node(node_index);
                return context->text_cursor_x(text, cursor, font_size, node ? context->node_font_id(*node) : Name());
            }

            virtual usize text_cursor_from_x(const String& text, f32 x, f32 font_size) const override
            {
                if(!context) return text.size();
                const Node* node = get_node(node_index);
                return context->text_cursor_from_x(text, x, font_size, node ? context->node_font_id(*node) : Name());
            }

            virtual bool is_popup_open(id_t popup_id) const override
            {
                return context && popup_id ? context->is_popup_open(popup_id) : false;
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

            virtual void draw_gradient_rect(const RectF& rect, const RectF& clip_rect,
                const Float4U& top_left, const Float4U& top_right, const Float4U& bottom_right, const Float4U& bottom_left) override
            {
                context->render_gradient_rect(rect, clip_rect, top_left, top_right, bottom_right, bottom_left);
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
                const Node* node = get_node(node_index);
                context->render_text(rect, clip_rect, text, font_size, color,
                    to_vg_text_alignment(horizontal_alignment), to_vg_text_alignment(vertical_alignment),
                    node ? context->node_font_id(*node) : Name());
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
            if(test_flags(image_flags, ImageFlag::nearest))
            {
                state.sampler = RHI::SamplerDesc(RHI::Filter::nearest, RHI::Filter::nearest, RHI::Filter::nearest,
                    RHI::TextureAddressMode::clamp,
                    RHI::TextureAddressMode::clamp,
                    RHI::TextureAddressMode::clamp);
            }
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

        void Context::render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color,
            VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment, const Name& font_id)
        {
            if(!text || !text[0]) return;
            RectF r = to_vg_rect(rect);
            RectF c = to_vg_rect(clip_rect);
            FontDesc font = resolve_font(font_id);
            VG::TextArrangeSection section;
            section.font_file = font.font;
            section.font_index = font.font_index;
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

        void Context::render_drag_drop_overlay()
        {
            if(!m_drag_drop.active || !m_drag_drop.type || m_layouts.size() != m_submitted_desc.nodes.size()) return;
            RectF surface_clip(0.0f, 0.0f, m_frame_desc.surface_size.x, m_frame_desc.surface_size.y);
            id_t hovered_target = hit_test_drag_drop_target(m_drag_drop.type, m_pointer_pos);
            for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
            {
                const Node& node = m_submitted_desc.nodes[i];
                if(!contains_name(node.drag_drop_target_types, m_drag_drop.type) || node.id == m_drag_drop.source_id) continue;
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

            if(!m_drag_drop.preview_built)
            {
                const c8* type_name = m_drag_drop.type.c_str();
                f32 width = max((f32)m_drag_drop.type.size() * 8.0f + 84.0f, 132.0f);
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

        void Context::render_dock_preview()
        {
            if(!dock_interaction_state().active_dock_panel_id || !dock_interaction_state().active_dock_panel_title_drag) return;
            DockPanelPersistentState* panel_state = find_dock_panel_state(dock_interaction_state().active_dock_space_id, dock_interaction_state().active_dock_panel_id);
            if(!panel_state || panel_state->mode != DockPanelMode::floating) return;

            id_t target_space_id = 0;
            u32 target_leaf = U32_MAX;
            DockDropDirection direction = DockDropDirection::none;
            if(!find_dock_drop_target(dock_interaction_state().active_dock_panel_id, m_pointer_pos, target_space_id, target_leaf, direction)) return;
            RectF target_rect(0.0f, 0.0f, 0.0f, 0.0f);
            bool empty_dock_space = target_leaf == U32_MAX;
            if(empty_dock_space)
            {
                for(usize i = 0; i < m_submitted_desc.nodes.size(); ++i)
                {
                    if(m_submitted_desc.nodes[i].id == target_space_id && dock_space_layout(m_submitted_desc.nodes[i]))
                    {
                        target_rect = m_layouts[i].rect;
                        break;
                    }
                }
            }
            else
            {
                DockSpaceState* dock_state = get_widget_state<DockSpaceState>(target_space_id);
                if(!dock_state || target_leaf >= dock_state->dock_nodes.size()) return;
                const DockTreeNode& leaf = dock_state->dock_nodes[target_leaf];
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
            if(popup_layer(node) && !popup_node_visible(node))
            {
                return;
            }
            if(tooltip_layer(node) && !tooltip_node_visible(node))
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
            ItemQueryState* query_state = get_widget_state<ItemQueryState>(node.id);
            if(query_state)
            {
                auto h = query_state->states.find(Name("gui.hovered"));
                hovered = h != query_state->states.end() && h->second.as<bool>() && *h->second.as<bool>();
                auto a = query_state->states.find(Name("gui.active"));
                active = a != query_state->states.end() && a->second.as<bool>() && *a->second.as<bool>();
                auto f = query_state->states.find(Name("gui.focused"));
                focused = f != query_state->states.end() && f->second.as<bool>() && *f->second.as<bool>();
            }

            NodeRenderState render_state{hovered, active, focused, m_frame_desc.surface_size, m_pointer_pos, m_frame_desc.delta_time, m_time};
            auto make_node_render_context = [&]()
            {
                ContextNodeRenderContext node_render_context;
                node_render_context.context = this;
                node_render_context.active_draw_list = m_active_draw_list;
                node_render_context.node_index = node_index;
                node_render_context.node_id = node.id;
                return node_render_context;
            };
            if(m_layouts[node_index].dock_panel_child)
            {
                ContextNodeRenderContext node_render_context = make_node_render_context();
                draw_dock_panel_chrome(node_render_context, node, render_state);
            }

            if(node.render_proxy.draw)
            {
                ContextNodeRenderContext node_render_context = make_node_render_context();
                node.render_proxy.draw(node_render_context, node, rect, clip, render_state, node.render_proxy.userdata);
            }

            if(dock_space_layout(node))
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
                {
                    ContextNodeRenderContext node_render_context = make_node_render_context();
                    draw_dock_space_splitters(node_render_context, node, render_state);
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
                if(!tab_item_layout(node) || m_layouts[node_index].tab_content_visible)
                {
                    for(u32 child = node.first_child; child != U32_MAX; child = m_submitted_desc.nodes[child].next_sibling)
                    {
                        render_node(child);
                    }
                }
            }
            if(node.render_proxy.draw_after_children)
            {
                ContextNodeRenderContext node_render_context = make_node_render_context();
                node.render_proxy.draw_after_children(node_render_context, node, rect, clip, render_state, node.render_proxy.userdata);
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
