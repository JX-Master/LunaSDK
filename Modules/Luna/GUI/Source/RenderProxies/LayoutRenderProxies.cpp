/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "LayoutRenderProxies.hpp"
#include "../Nodes/LayoutNodes.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static bool point_in_rect_local(const Float2U& pos, const RectF& rect)
        {
            return pos.x >= rect.offset_x && pos.x <= rect.offset_x + rect.width &&
                pos.y >= rect.offset_y && pos.y <= rect.offset_y + rect.height;
        }

        static RectF smooth_rect(const RectF& a, const RectF& b, f32 t)
        {
            t = clamp(t, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t);
            return RectF(
                a.offset_x + (b.offset_x - a.offset_x) * t,
                a.offset_y + (b.offset_y - a.offset_y) * t,
                a.width + (b.width - a.width) * t,
                a.height + (b.height - a.height) * t);
        }

        static bool item_query_bool(NodeRenderContext& ctx, id_t item_id, const Name& state_name)
        {
            ItemQueryState* query_state = ctx.get_widget_state<ItemQueryState>(item_id);
            if(!query_state) return false;
            auto iter = query_state->states.find(state_name);
            return iter != query_state->states.end() && iter->second.as<bool>() && *iter->second.as<bool>();
        }

        static void draw_default_tab_item_background(NodeRenderContext& ctx, const Node& node, const NodeRenderLayout& layout,
            bool draw_selected_background)
        {
            RectF rect = layout.tab_header_rect;
            RectF clip = layout.tab_header_clip_rect;
            if(rect.width <= 0.0f || rect.height <= 0.0f) return;

            bool hovered = item_query_bool(ctx, node.id, Name("gui.hovered"));
            bool active = item_query_bool(ctx, node.id, Name("gui.active"));
            bool selected = layout.tab_content_visible;
            const TabItemNode* tab = tab_item_node(node);
            if(!tab) return;
            bool button = test_flags(tab->flags, TabItemFlag::button);
            if(selected && !button && !draw_selected_background)
            {
                return;
            }
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
            ctx.draw_rect(rect, clip, color, radius);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + max(rect.height - radius, 0.0f), rect.width, min(radius, rect.height)),
                clip, color, 0.0f);
            if(selected && draw_selected_background)
            {
                ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, rect.width, 2.0f), clip, Float4U(0.34f, 0.60f, 0.92f, 1.0f), 1.0f);
            }
        }

        static void draw_default_tab_item_foreground(NodeRenderContext& ctx, const Node& node, const NodeRenderLayout& layout,
            const NodeRenderState& state)
        {
            RectF rect = layout.tab_header_rect;
            RectF clip = layout.tab_header_clip_rect;
            if(rect.width <= 0.0f || rect.height <= 0.0f) return;

            const TabItemNode* tab = tab_item_node(node);
            if(!tab) return;

            f32 left = rect.offset_x + 9.0f;
            f32 right = rect.offset_x + rect.width - 8.0f;
            if(test_flags(tab->flags, TabItemFlag::unsaved_document))
            {
                f32 dot = 6.0f;
                ctx.draw_circle(RectF(left, rect.offset_y + (rect.height - dot) * 0.5f, dot, dot), clip, Float4U(0.95f, 0.64f, 0.28f, 1.0f));
                left += dot + 6.0f;
            }
            if(layout.tab_close_rect.width > 0.0f)
            {
                bool close_hovered = point_in_rect_local(state.pointer_position, layout.tab_close_rect);
                if(close_hovered)
                {
                    ctx.draw_rect(layout.tab_close_rect, clip, Float4U(0.46f, 0.18f, 0.18f, 1.0f), 4.0f);
                }
                ctx.draw_text(layout.tab_close_rect, clip, "X", 12.0f, Color::white(), TextAlignment::center);
                right = min(right, layout.tab_close_rect.offset_x - 4.0f);
            }
            ctx.draw_text(RectF(left, rect.offset_y, max(right - left, 1.0f), rect.height),
                clip, node.text.c_str(), 15.0f, Color::white(), TextAlignment::begin);
        }

        static void draw_default_tab_bar_headers(NodeRenderContext& ctx, const Node& node, const NodeRenderState& state)
        {
            TabBarState* tab_state = ctx.get_widget_state<TabBarState>(node.id);
            RectF selected_rect(0.0f, 0.0f, 0.0f, 0.0f);
            RectF selected_clip(0.0f, 0.0f, 0.0f, 0.0f);
            bool has_selected_rect = false;

            for(u32 child = node.first_child; child != U32_MAX;)
            {
                const Node* child_node = ctx.get_node(child);
                if(!child_node) break;
                if(tab_item_layout(*child_node))
                {
                    NodeRenderLayout child_layout;
                    if(ctx.get_node_render_layout(child, child_layout))
                    {
                        draw_default_tab_item_background(ctx, *child_node, child_layout, false);
                        if(tab_state && child_node->id == tab_state->tab_selected_id && child_layout.tab_content_visible &&
                            child_layout.tab_header_rect.width > 0.0f && child_layout.tab_header_rect.height > 0.0f)
                        {
                            selected_rect = child_layout.tab_header_rect;
                            selected_clip = child_layout.tab_header_clip_rect;
                            has_selected_rect = true;
                        }
                    }
                }
                child = child_node->next_sibling;
            }

            if(has_selected_rect)
            {
                RectF selection_rect = selected_rect;
                TabBarAnimationState* animation_state = ctx.get_widget_state<TabBarAnimationState>(node.id);
                if(animation_state && animation_state->selection_rect_initialized)
                {
                    selection_rect = smooth_rect(animation_state->selection_rect, selected_rect, clamp(state.delta_time * 14.0f, 0.0f, 1.0f));
                }
                Ref<TabBarAnimationState> next_animation_state = ctx.get_or_create_widget_state<TabBarAnimationState>(node.id);
                next_animation_state->selection_rect = selection_rect;
                next_animation_state->selection_rect_initialized = true;

                Float4U color = Float4U(0.17f, 0.27f, 0.42f, 1.0f);
                f32 radius = 5.0f;
                ctx.draw_rect(selection_rect, selected_clip, color, radius);
                ctx.draw_rect(RectF(selection_rect.offset_x, selection_rect.offset_y + max(selection_rect.height - radius, 0.0f),
                    selection_rect.width, min(radius, selection_rect.height)), selected_clip, color, 0.0f);
                ctx.draw_rect(RectF(selection_rect.offset_x, selection_rect.offset_y, selection_rect.width, 2.0f),
                    selected_clip, Float4U(0.34f, 0.60f, 0.92f, 1.0f), 1.0f);
            }

            for(u32 child = node.first_child; child != U32_MAX;)
            {
                const Node* child_node = ctx.get_node(child);
                if(!child_node) break;
                if(tab_item_layout(*child_node))
                {
                    NodeRenderLayout child_layout;
                    if(ctx.get_node_render_layout(child, child_layout))
                    {
                        draw_default_tab_item_foreground(ctx, *child_node, child_layout, state);
                    }
                }
                child = child_node->next_sibling;
            }
        }

        static void draw_default_tab_item(NodeRenderContext& ctx, const Node& node, const RectF&, const RectF&,
            const NodeRenderState& state, void*)
        {
            const Node* parent = node.parent == U32_MAX ? nullptr : ctx.get_node(node.parent);
            if(parent && tab_bar_layout(*parent)) return;

            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            draw_default_tab_item_background(ctx, node, layout, true);
            draw_default_tab_item_foreground(ctx, node, layout, state);
        }

        static void draw_default_tab_bar_scroll_buttons(NodeRenderContext& ctx, const Node& node, const NodeRenderLayout& layout,
            const NodeRenderState& render_state)
        {
            if(!layout.tab_scrollable) return;
            Ref<TabBarState> state_ref = ctx.get_or_create_widget_state<TabBarState>(node.id);
            TabBarState& tab_state = *state_ref;
            TabInteractionState* tab_interaction = ctx.get_widget_state<TabInteractionState>(0);
            auto draw_button = [&](const RectF& rect, bool left) {
                bool enabled = left ? tab_state.tab_scroll_x > 0.5f : tab_state.tab_scroll_x < layout.tab_scroll_max - 0.5f;
                bool hovered = enabled && point_in_rect_local(render_state.pointer_position, rect);
                bool active = enabled && tab_interaction && tab_interaction->active_tab_scroll_id == node.id && tab_interaction->active_tab_scroll_left == left;
                Float4U color = !enabled ? Float4U(0.09f, 0.10f, 0.12f, 0.82f) :
                    (active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                        (hovered ? Float4U(0.18f, 0.25f, 0.35f, 1.0f) : Float4U(0.11f, 0.14f, 0.19f, 0.95f)));
                ctx.draw_rect(rect, layout.clip_rect, color, 4.0f);
                Float4U arrow_color = enabled ? Float4U(1.0f) : Float4U(0.45f, 0.48f, 0.52f, 1.0f);
                f32 cx = rect.offset_x + rect.width * 0.5f;
                f32 cy = rect.offset_y + rect.height * 0.5f;
                if(left)
                {
                    ctx.draw_line(Float2U(cx + 4.0f, cy - 6.0f), Float2U(cx - 3.0f, cy), layout.clip_rect, arrow_color, 1.8f);
                    ctx.draw_line(Float2U(cx - 3.0f, cy), Float2U(cx + 4.0f, cy + 6.0f), layout.clip_rect, arrow_color, 1.8f);
                }
                else
                {
                    ctx.draw_line(Float2U(cx - 4.0f, cy - 6.0f), Float2U(cx + 3.0f, cy), layout.clip_rect, arrow_color, 1.8f);
                    ctx.draw_line(Float2U(cx + 3.0f, cy), Float2U(cx - 4.0f, cy + 6.0f), layout.clip_rect, arrow_color, 1.8f);
                }
            };
            draw_button(layout.tab_scroll_left_rect, true);
            draw_button(layout.tab_scroll_right_rect, false);
        }

        static void draw_default_tab_bar_after_children(NodeRenderContext& ctx, const Node& node, const RectF&, const RectF&,
            const NodeRenderState& state, void*)
        {
            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            draw_default_tab_bar_scroll_buttons(ctx, node, layout, state);
        }

        static void draw_default_scroll_view(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            ctx.draw_rect(rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.scroll_view.background"), Float4U(0.10f, 0.12f, 0.14f, 0.92f)),
                style_f32(ctx, node, Name("gui.scroll_view.radius"), 6.0f));
        }

        static f32 render_scroll_max_x(const NodeRenderLayout& layout)
        {
            return max(layout.scroll_content_size.x - layout.scroll_viewport_size.x, 0.0f);
        }

        static f32 render_scroll_max_y(const NodeRenderLayout& layout)
        {
            return max(layout.scroll_content_size.y - layout.scroll_viewport_size.y, 0.0f);
        }

        static bool render_scroll_has_vertical_bar(const NodeRenderLayout& layout)
        {
            return layout.scroll_has_vertical && render_scroll_max_y(layout) > 0.0f;
        }

        static bool render_scroll_has_horizontal_bar(const NodeRenderLayout& layout)
        {
            return layout.scroll_has_horizontal && render_scroll_max_x(layout) > 0.0f;
        }

        static RectF render_scroll_vertical_track_rect(const NodeRenderLayout& layout)
        {
            f32 size = scroll_bar_size();
            f32 margin = scroll_bar_margin();
            f32 bottom_reserved = render_scroll_has_horizontal_bar(layout) ? size + margin : 0.0f;
            return RectF(
                layout.rect.offset_x + max(layout.rect.width - size - margin, 0.0f),
                layout.rect.offset_y + margin,
                size,
                max(layout.rect.height - margin * 2.0f - bottom_reserved, 1.0f));
        }

        static RectF render_scroll_horizontal_track_rect(const NodeRenderLayout& layout)
        {
            f32 size = scroll_bar_size();
            f32 margin = scroll_bar_margin();
            f32 right_reserved = render_scroll_has_vertical_bar(layout) ? size + margin : 0.0f;
            return RectF(
                layout.rect.offset_x + margin,
                layout.rect.offset_y + max(layout.rect.height - size - margin, 0.0f),
                max(layout.rect.width - margin * 2.0f - right_reserved, 1.0f),
                size);
        }

        static RectF render_scroll_vertical_thumb_rect(const NodeRenderLayout& layout, const ScrollState& state)
        {
            RectF track = render_scroll_vertical_track_rect(layout);
            f32 ratio = layout.scroll_content_size.y > 0.0f ? clamp(layout.scroll_viewport_size.y / layout.scroll_content_size.y, 0.0f, 1.0f) : 1.0f;
            f32 thumb_height = min(max(track.height * ratio, min(scroll_min_thumb_size(), track.height)), track.height);
            f32 travel = max(track.height - thumb_height, 0.0f);
            f32 t = render_scroll_max_y(layout) > 0.0f ? clamp(state.scroll_y / render_scroll_max_y(layout), 0.0f, 1.0f) : 0.0f;
            return RectF(track.offset_x, track.offset_y + travel * t, track.width, thumb_height);
        }

        static RectF render_scroll_horizontal_thumb_rect(const NodeRenderLayout& layout, const ScrollState& state)
        {
            RectF track = render_scroll_horizontal_track_rect(layout);
            f32 ratio = layout.scroll_content_size.x > 0.0f ? clamp(layout.scroll_viewport_size.x / layout.scroll_content_size.x, 0.0f, 1.0f) : 1.0f;
            f32 thumb_width = min(max(track.width * ratio, min(scroll_min_thumb_size(), track.width)), track.width);
            f32 travel = max(track.width - thumb_width, 0.0f);
            f32 t = render_scroll_max_x(layout) > 0.0f ? clamp(state.scroll_x / render_scroll_max_x(layout), 0.0f, 1.0f) : 0.0f;
            return RectF(track.offset_x + travel * t, track.offset_y, thumb_width, track.height);
        }

        static RectF render_scroll_display_thumb_rect(const RectF& thumb, bool vertical, f32 hover_amount)
        {
            f32 full_size = vertical ? thumb.width : thumb.height;
            f32 thickness = scroll_bar_collapsed_size() + (full_size - scroll_bar_collapsed_size()) * clamp(hover_amount, 0.0f, 1.0f);
            if(vertical)
            {
                return RectF(thumb.offset_x + (full_size - thickness) * 0.5f, thumb.offset_y, thickness, thumb.height);
            }
            return RectF(thumb.offset_x, thumb.offset_y + (full_size - thickness) * 0.5f, thumb.width, thickness);
        }

        static void draw_default_scroll_view_after_children(NodeRenderContext& ctx, const Node& node, const RectF&, const RectF&,
            const NodeRenderState& render_state, void*)
        {
            NodeRenderLayout layout;
            if(!ctx.get_node_render_layout(ctx.current_node_index(), layout)) return;
            if(!render_scroll_has_vertical_bar(layout) && !render_scroll_has_horizontal_bar(layout)) return;

            bool reserved_scroll_bar = scroll_bar_reserved(node);
            Ref<ScrollState> state_ref = ctx.get_or_create_widget_state<ScrollState>(node.id);
            ScrollState& state = *state_ref;
            bool hovered = false;
            bool visible_for_hover = reserved_scroll_bar || scroll_bar_visible_for_input(state);
            if(visible_for_hover && render_scroll_has_vertical_bar(layout) && point_in_rect_local(render_state.pointer_position, render_scroll_vertical_track_rect(layout)))
            {
                hovered = true;
            }
            if(visible_for_hover && render_scroll_has_horizontal_bar(layout) && point_in_rect_local(render_state.pointer_position, render_scroll_horizontal_track_rect(layout)))
            {
                hovered = true;
            }
            ScrollbarInteractionState* scrollbar_interaction = ctx.get_widget_state<ScrollbarInteractionState>(0);
            bool active = scrollbar_interaction && scrollbar_interaction->active_scrollbar_id == node.id;
            if(reserved_scroll_bar)
            {
                state.scrollbar_visibility = 1.0f;
                state.scrollbar_hover = 1.0f;
                state.scrollbar_idle_time = 0.0f;
            }
            else if(hovered || active)
            {
                state.scrollbar_idle_time = 0.0f;
            }
            else
            {
                state.scrollbar_idle_time += render_state.delta_time;
            }
            if(!reserved_scroll_bar)
            {
                f32 target_visibility = (hovered || active || state.scrollbar_idle_time < scroll_bar_visible_time()) ? 1.0f : 0.0f;
                f32 target_hover = (hovered || active) ? 1.0f : 0.0f;
                f32 blend = clamp(render_state.delta_time * 12.0f, 0.0f, 1.0f);
                state.scrollbar_visibility += (target_visibility - state.scrollbar_visibility) * blend;
                state.scrollbar_hover += (target_hover - state.scrollbar_hover) * blend;
            }
            f32 visibility = clamp(state.scrollbar_visibility, 0.0f, 1.0f);
            if(visibility <= 0.01f && !active) return;
            f32 hover_amount = clamp(state.scrollbar_hover, 0.0f, 1.0f);
            const RectF& clip = layout.clip_rect;
            f32 radius = scroll_bar_size() * 0.5f;
            Float4U track_color(0.02f, 0.025f, 0.03f, visibility * (reserved_scroll_bar ? 0.42f : hover_amount * 0.52f));
            Float4U thumb_color(0.58f, 0.68f, 0.80f, visibility * (reserved_scroll_bar ? (hovered || active ? 1.0f : 0.86f) : (0.72f + hover_amount * 0.28f)));

            if(render_scroll_has_vertical_bar(layout))
            {
                RectF track = render_scroll_vertical_track_rect(layout);
                RectF thumb = render_scroll_display_thumb_rect(render_scroll_vertical_thumb_rect(layout, state), true, hover_amount);
                if(track_color.w > 0.01f)
                {
                    ctx.draw_rect(track, clip, track_color, radius);
                }
                ctx.draw_rect(thumb, clip, thumb_color, thumb.width * 0.5f);
            }
            if(render_scroll_has_horizontal_bar(layout))
            {
                RectF track = render_scroll_horizontal_track_rect(layout);
                RectF thumb = render_scroll_display_thumb_rect(render_scroll_horizontal_thumb_rect(layout, state), false, hover_amount);
                if(track_color.w > 0.01f)
                {
                    ctx.draw_rect(track, clip, track_color, radius);
                }
                ctx.draw_rect(thumb, clip, thumb_color, thumb.height * 0.5f);
            }
        }

        static void draw_default_window(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const WindowNode* window = cast_node<WindowNode>(node);
            if(!window) return;
            ctx.draw_rect(rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.window.background"), Float4U(0.10f, 0.12f, 0.14f, 0.92f)),
                style_f32(ctx, node, Name("gui.window.radius"), 6.0f));
            if(!window->open) return;
            RectF title_rect(rect.offset_x, rect.offset_y, rect.width, WindowNode::title_bar_height());
            ctx.draw_rect(title_rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.window.title_background"), Float4U(0.13f, 0.17f, 0.22f, 1.0f)),
                style_f32(ctx, node, Name("gui.window.radius"), 6.0f));
            ctx.draw_text(RectF(rect.offset_x + 10.0f, rect.offset_y, max(rect.width - 46.0f, 1.0f), WindowNode::title_bar_height()),
                clip_rect, node.text.c_str(), style_f32(ctx, node, Name("gui.window.title_font_size"), 15.0f),
                style_f32x4(ctx, node, Name("gui.window.title_color"), Float4U(1.0f)), TextAlignment::begin);
            RectF close = WindowNode::close_rect(rect);
            bool close_hovered = point_in_rect_local(state.pointer_position, close);
            ctx.draw_rect(close, clip_rect,
                close_hovered ?
                    style_f32x4(ctx, node, Name("gui.window.close_hovered"), Float4U(0.55f, 0.18f, 0.18f, 1.0f)) :
                    style_f32x4(ctx, node, Name("gui.window.close_background"), Float4U(0.23f, 0.27f, 0.33f, 1.0f)),
                style_f32(ctx, node, Name("gui.window.close_radius"), 4.0f));
            ctx.draw_text(close, clip_rect, "X", style_f32(ctx, node, Name("gui.window.close_font_size"), 14.0f),
                style_f32x4(ctx, node, Name("gui.window.close_color"), Float4U(1.0f)), TextAlignment::center);
        }

        static void draw_default_popup(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            ctx.draw_rect(rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.popup.background"), Float4U(0.08f, 0.10f, 0.13f, 0.98f)),
                style_f32(ctx, node, Name("gui.popup.radius"), 5.0f));
        }

        static void draw_default_tooltip(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            ctx.draw_rect(rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.tooltip.background"), Float4U(0.05f, 0.06f, 0.07f, 0.97f)),
                style_f32(ctx, node, Name("gui.tooltip.radius"), 4.0f));
            Float4U border = style_f32x4(ctx, node, Name("gui.tooltip.border"), Float4U(0.28f, 0.33f, 0.40f, 1.0f));
            f32 width = style_f32(ctx, node, Name("gui.tooltip.border_width"), 1.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, rect.width, width), clip_rect, border, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + max(rect.height - width, 0.0f), rect.width, width), clip_rect, border, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, width, rect.height), clip_rect, border, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x + max(rect.width - width, 0.0f), rect.offset_y, width, rect.height), clip_rect, border, 0.0f);
        }

        static void draw_default_menu_bar(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            ctx.draw_rect(rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.menu_bar.background"), Float4U(0.08f, 0.10f, 0.13f, 0.92f)), 0.0f);
            f32 border_width = style_f32(ctx, node, Name("gui.menu_bar.border_width"), 1.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + max(rect.height - border_width, 0.0f), rect.width, border_width),
                clip_rect, style_f32x4(ctx, node, Name("gui.menu_bar.border"), Float4U(0.20f, 0.24f, 0.30f, 1.0f)), 0.0f);
        }

        static void draw_default_tab_bar(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            ctx.draw_rect(rect, clip_rect,
                style_f32x4(ctx, node, Name("gui.tab_bar.background"), Float4U(0.08f, 0.10f, 0.13f, 0.70f)),
                style_f32(ctx, node, Name("gui.tab_bar.radius"), 4.0f));
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + style_f32(ctx, node, Name("gui.tab_bar.header_line_y"), 31.0f), rect.width,
                style_f32(ctx, node, Name("gui.tab_bar.header_line_width"), 1.0f)),
                clip_rect, style_f32x4(ctx, node, Name("gui.tab_bar.header_line"), Float4U(0.22f, 0.27f, 0.34f, 1.0f)), 0.0f);
            draw_default_tab_bar_headers(ctx, node, state);
        }

        RenderProxyDesc default_scroll_view_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_scroll_view;
            desc.draw_after_children = draw_default_scroll_view_after_children;
            return desc;
        }

        RenderProxyDesc default_window_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_window;
            return desc;
        }

        RenderProxyDesc default_popup_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_popup;
            return desc;
        }

        RenderProxyDesc default_tooltip_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_tooltip;
            return desc;
        }

        RenderProxyDesc default_menu_bar_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_menu_bar;
            return desc;
        }

        RenderProxyDesc default_dock_space_render_proxy()
        {
            RenderProxyDesc desc;
            return desc;
        }

        RenderProxyDesc default_tab_bar_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_tab_bar;
            desc.draw_after_children = draw_default_tab_bar_after_children;
            return desc;
        }

        RenderProxyDesc default_tab_item_render_proxy()
        {
            RenderProxyDesc desc;
            desc.draw = draw_default_tab_item;
            return desc;
        }
    }
}
