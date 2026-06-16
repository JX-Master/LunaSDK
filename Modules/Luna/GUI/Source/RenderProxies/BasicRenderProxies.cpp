/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "BasicRenderProxies.hpp"
#include "../Nodes/BasicNodes.hpp"
#include "../Nodes/ButtonGroupNodes.hpp"
#include "../GUI.hpp"
#include "../../State.hpp"
#include <cstdio>

namespace Luna
{
    namespace GUI
    {
#define LUNA_GUI_STYLE_F32(entry_name, value, category) {Name(entry_name), StyleValueType::f32_1, StyleValue::f32_1(value), nullptr, category, nullptr}
#define LUNA_GUI_STYLE_F32X2(entry_name, value, category) {Name(entry_name), StyleValueType::f32_2, StyleValue::f32_2(value), nullptr, category, nullptr}
#define LUNA_GUI_STYLE_F32X4(entry_name, value, category) {Name(entry_name), StyleValueType::f32_4, StyleValue::f32_4(value), nullptr, category, nullptr}
#define LUNA_GUI_STYLE_NAME(entry_name, value, category) {Name(entry_name), StyleValueType::name, StyleValue::name(Name(value)), nullptr, category, nullptr}

        static Float4U disabled_alpha(NodeRenderContext& ctx, const Node& node, const Float4U& color)
        {
            f32 alpha = style_f32(ctx, node, Name("gui.disabled_alpha"), 0.52f);
            return Float4U(color.x, color.y, color.z, color.w * alpha);
        }

        static void draw_default_button(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            Float4U default_color = style_f32x4(ctx, node, Name("gui.button.background"), Float4U(0.18f, 0.28f, 0.45f, 1.0f));
            Float4U hovered_color = style_f32x4(ctx, node, Name("gui.button.background_hovered"), Float4U(0.26f, 0.43f, 0.72f, 1.0f));
            Float4U active_color = style_f32x4(ctx, node, Name("gui.button.background_active"), Float4U(0.20f, 0.36f, 0.62f, 1.0f));
            Float4U disabled_background = style_f32x4(ctx, node, Name("gui.button.background_disabled"), Float4U(0.13f, 0.16f, 0.20f, 1.0f));
            Float4U target_color = !state.enabled ? disabled_background : (state.active ? active_color : (state.hovered ? hovered_color : default_color));
            Float4U color = target_color;
            ButtonAnimationState* animation_state = ctx.get_widget_state<ButtonAnimationState>(node.id);
            if(animation_state && animation_state->initialized)
            {
                f32 blend = clamp(state.delta_time * style_f32(ctx, node, Name("gui.button.animation_speed"), 14.0f), 0.0f, 1.0f);
                color = smooth_color(animation_state->color, target_color, blend);
            }
            Ref<ButtonAnimationState> next_animation_state = ctx.get_or_create_widget_state<ButtonAnimationState>(node.id);
            next_animation_state->color = color;
            next_animation_state->initialized = true;
            f32 radius = style_f32(ctx, node, Name("gui.button.radius"), 5.0f);
            ctx.draw_rect(rect, clip_rect, color, radius);
        }

        static void draw_default_button_label(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            f32 font_size = style_f32(ctx, node, Name("gui.button.font_size"), 16.0f);
            Float4U text_color = state.enabled ?
                style_f32x4(ctx, node, Name("gui.button.text_color"), Float4U(1.0f)) :
                style_f32x4(ctx, node, Name("gui.button.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
            ctx.draw_text(rect, clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::center);
        }

        static void draw_default_text(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const TextNode* text_node = cast_node<TextNode>(node);
            f32 default_font_size = text_node ? text_node->font_size : 16.0f;
            Float4U default_color = text_node ? text_node->color : Float4U(1.0f);
            f32 font_size = style_f32(ctx, node, Name("gui.text.font_size"), default_font_size);
            Float4U color = state.enabled ?
                style_f32x4(ctx, node, Name("gui.text.color"), default_color) :
                style_f32x4(ctx, node, Name("gui.text.disabled"), disabled_alpha(ctx, node, default_color));
            ctx.draw_text(rect, clip_rect, node.text.c_str(), font_size, color, TextAlignment::begin);
        }

        static void draw_default_progress_bar(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState&, void*)
        {
            const ProgressBarNode* progress_bar = cast_node<ProgressBarNode>(node);
            if(!progress_bar) return;

            f32 border_size = style_f32(ctx, node, Name("gui.progress_bar.border_size"), 1.0f);
            f32 radius = style_f32(ctx, node, Name("gui.progress_bar.radius"), min(rect.height * 0.5f, 5.0f));
            Float4U border_color = style_f32x4(ctx, node, Name("gui.progress_bar.border"), Float4U(0.25f, 0.29f, 0.35f, 1.0f));
            Float4U background = style_f32x4(ctx, node, Name("gui.progress_bar.background"), Float4U(0.07f, 0.08f, 0.10f, 1.0f));
            Float4U fill = style_f32x4(ctx, node, Name("gui.progress_bar.fill"), Float4U(0.20f, 0.36f, 0.62f, 1.0f));

            ctx.draw_rect(rect, clip_rect, border_color, radius);
            RectF inner(
                rect.offset_x + border_size,
                rect.offset_y + border_size,
                max(rect.width - border_size * 2.0f, 1.0f),
                max(rect.height - border_size * 2.0f, 1.0f));
            f32 inner_radius = max(radius - border_size, 0.0f);
            ctx.draw_rect(inner, clip_rect, background, inner_radius);

            f32 fraction = clamp(progress_bar->fraction, 0.0f, 1.0f);
            if(fraction > 0.0f)
            {
                RectF fill_rect(inner.offset_x, inner.offset_y, max(inner.width * fraction, 1.0f), inner.height);
                f32 max_x = inner.offset_x + inner.width;
                if(fill_rect.offset_x + fill_rect.width > max_x)
                {
                    fill_rect.width = max(max_x - fill_rect.offset_x, 1.0f);
                }
                bool full = fraction >= 0.999f;
                ctx.draw_rect_corners(fill_rect, clip_rect, fill, inner_radius, true, full, full, true);
            }

            const c8* overlay = progress_bar->has_overlay ? progress_bar->overlay.c_str() : nullptr;
            c8 percentage[32];
            if(!overlay)
            {
                snprintf(percentage, sizeof(percentage), "%.0f%%", fraction * 100.0f);
                overlay = percentage;
            }
            f32 font_size = style_f32(ctx, node, Name("gui.progress_bar.font_size"), 14.0f);
            Float4U text_color = style_f32x4(ctx, node, Name("gui.progress_bar.text_color"), Float4U(1.0f));
            ctx.draw_text(RectF(inner.offset_x + 6.0f, inner.offset_y, max(inner.width - 12.0f, 1.0f), inner.height),
                clip_rect, overlay, font_size, text_color, TextAlignment::center);
        }

        static void draw_default_selectable(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const SelectableNode* selectable = cast_node<SelectableNode>(node);
            if(!selectable) return;
            if(selectable->selected || state.hovered || state.active)
            {
                Float4U active_color = style_f32x4(ctx, node, Name("gui.selectable.background_active"), Float4U(0.20f, 0.36f, 0.62f, 1.0f));
                Float4U selected_color = style_f32x4(ctx, node, Name("gui.selectable.background_selected"), Float4U(0.16f, 0.25f, 0.38f, 1.0f));
                Float4U hovered_color = style_f32x4(ctx, node, Name("gui.selectable.background_hovered"), Float4U(0.20f, 0.30f, 0.44f, 1.0f));
                Float4U color = state.enabled ? (state.active ? active_color : (state.hovered ? hovered_color : selected_color)) :
                    disabled_alpha(ctx, node, selected_color);
                ctx.draw_rect(rect, clip_rect, color, style_f32(ctx, node, Name("gui.selectable.radius"), 4.0f));
            }
            if(!selectable->label_layout)
            {
                f32 font_size = style_f32(ctx, node, Name("gui.selectable.font_size"), 15.0f);
                Float4U text_color = state.enabled ?
                    style_f32x4(ctx, node, Name("gui.selectable.text_color"), Float4U(1.0f)) :
                    style_f32x4(ctx, node, Name("gui.selectable.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
                ctx.draw_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height),
                    clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
            }
        }

        static void draw_default_checkbox(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const CheckboxNode* checkbox = cast_node<CheckboxNode>(node);
            if(!checkbox) return;
            f32 box_size = style_f32(ctx, node, Name("gui.checkbox.box_size"), 18.0f);
            RectF box(rect.offset_x + 2.0f, rect.offset_y + max((rect.height - box_size) * 0.5f, 0.0f), box_size, box_size);
            bool checked = checkbox->value && *checkbox->value;
            Float4U border_color = !state.enabled ?
                style_f32x4(ctx, node, Name("gui.checkbox.border_disabled"), Float4U(0.18f, 0.21f, 0.26f, 1.0f)) :
                (state.hovered ?
                    style_f32x4(ctx, node, Name("gui.checkbox.border_hovered"), Float4U(0.34f, 0.39f, 0.46f, 1.0f)) :
                    style_f32x4(ctx, node, Name("gui.checkbox.border"), Float4U(0.25f, 0.29f, 0.35f, 1.0f)));
            Float4U checked_color = style_f32x4(ctx, node, Name("gui.checkbox.background_checked"), Float4U(0.22f, 0.55f, 0.32f, 1.0f));
            Float4U unchecked_color = style_f32x4(ctx, node, Name("gui.checkbox.background"), Float4U(0.18f, 0.20f, 0.23f, 1.0f));
            if(!state.enabled)
            {
                checked_color = style_f32x4(ctx, node, Name("gui.checkbox.background_checked_disabled"), disabled_alpha(ctx, node, checked_color));
                unchecked_color = style_f32x4(ctx, node, Name("gui.checkbox.background_disabled"), Float4U(0.12f, 0.14f, 0.17f, 1.0f));
            }
            f32 radius = style_f32(ctx, node, Name("gui.checkbox.radius"), 3.0f);
            ctx.draw_rect(RectF(box.offset_x - 1.0f, box.offset_y - 1.0f, box.width + 2.0f, box.height + 2.0f), clip_rect, border_color, radius + 1.0f);
            ctx.draw_rect(box, clip_rect, checked ? checked_color : unchecked_color, radius);
            if(checked)
            {
                Float4U check_color = state.enabled ?
                    style_f32x4(ctx, node, Name("gui.checkbox.check_color"), Float4U(1.0f)) :
                    style_f32x4(ctx, node, Name("gui.checkbox.check_disabled"), Float4U(0.70f, 0.74f, 0.80f, 1.0f));
                f32 scale = box_size / 18.0f;
                f32 stroke = style_f32(ctx, node, Name("gui.checkbox.check_width"), 2.4f) * scale;
                ctx.draw_line(Float2U(box.offset_x + 4.0f * scale, box.offset_y + 9.5f * scale),
                    Float2U(box.offset_x + 7.5f * scale, box.offset_y + 13.0f * scale), clip_rect, check_color, stroke);
                ctx.draw_line(Float2U(box.offset_x + 7.5f * scale, box.offset_y + 13.0f * scale),
                    Float2U(box.offset_x + 14.5f * scale, box.offset_y + 5.5f * scale), clip_rect, check_color, stroke);
            }
            if(!checkbox->label_layout)
            {
                f32 label_offset = style_f32(ctx, node, Name("gui.checkbox.label_offset"), 28.0f);
                f32 font_size = style_f32(ctx, node, Name("gui.checkbox.font_size"), 16.0f);
                Float4U text_color = state.enabled ?
                    style_f32x4(ctx, node, Name("gui.checkbox.text_color"), Float4U(1.0f)) :
                    style_f32x4(ctx, node, Name("gui.checkbox.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
                ctx.draw_text(RectF(rect.offset_x + label_offset, rect.offset_y, max(rect.width - label_offset, 1.0f), rect.height),
                    clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
            }
        }

        static void draw_default_toggle_switch(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const ToggleSwitchNode* toggle = cast_node<ToggleSwitchNode>(node);
            if(!toggle) return;
            bool checked = toggle->value && *toggle->value;
            f32 target = checked ? 1.0f : 0.0f;
            f32 animation = target;
            SwitchAnimationState* switch_state = ctx.get_widget_state<SwitchAnimationState>(node.id);
            if(switch_state && switch_state->initialized)
            {
                animation = switch_state->animation;
            }
            f32 speed = style_f32(ctx, node, Name("gui.switch.animation_speed"), 14.0f);
            f32 blend = clamp(state.delta_time * speed, 0.0f, 1.0f);
            animation += (target - animation) * blend;
            animation = clamp(animation, 0.0f, 1.0f);
            Ref<SwitchAnimationState> next_state = ctx.get_or_create_widget_state<SwitchAnimationState>(node.id);
            next_state->animation = animation;
            next_state->initialized = true;

            Float2U track_size = style_f32x2(ctx, node, Name("gui.switch.track_size"), Float2U(44.0f, 22.0f));
            RectF track(rect.offset_x + 2.0f, rect.offset_y + max((rect.height - track_size.y) * 0.5f, 0.0f), track_size.x, track_size.y);
            Float4U off_track = !state.enabled ?
                style_f32x4(ctx, node, Name("gui.switch.track_disabled"), Float4U(0.11f, 0.12f, 0.14f, 1.0f)) :
                (state.hovered ?
                    style_f32x4(ctx, node, Name("gui.switch.off_track_hovered"), Float4U(0.18f, 0.20f, 0.23f, 1.0f)) :
                    style_f32x4(ctx, node, Name("gui.switch.off_track"), Float4U(0.12f, 0.14f, 0.16f, 1.0f)));
            Float4U on_track = !state.enabled ?
                style_f32x4(ctx, node, Name("gui.switch.track_checked_disabled"), Float4U(0.17f, 0.23f, 0.20f, 1.0f)) :
                (state.hovered ?
                    style_f32x4(ctx, node, Name("gui.switch.on_track_hovered"), Float4U(0.25f, 0.62f, 0.38f, 1.0f)) :
                    style_f32x4(ctx, node, Name("gui.switch.on_track"), Float4U(0.20f, 0.55f, 0.32f, 1.0f)));
            ctx.draw_rect(track, clip_rect, smooth_color(off_track, on_track, animation), track.height * 0.5f);

            f32 knob_size = style_f32(ctx, node, Name("gui.switch.knob_size"), 18.0f);
            f32 knob_margin = style_f32(ctx, node, Name("gui.switch.knob_margin"), 2.0f);
            f32 knob_x = track.offset_x + knob_margin + (track.width - knob_size - knob_margin * 2.0f) * animation;
            RectF knob(knob_x, track.offset_y + knob_margin, knob_size, knob_size);
            Float4U off_knob = style_f32x4(ctx, node, Name("gui.switch.off_knob"), Float4U(0.78f, 0.80f, 0.84f, 1.0f));
            Float4U on_knob = style_f32x4(ctx, node, Name("gui.switch.on_knob"), Float4U(1.0f));
            if(!state.enabled)
            {
                off_knob = style_f32x4(ctx, node, Name("gui.switch.knob_disabled"), Float4U(0.45f, 0.48f, 0.54f, 1.0f));
                on_knob = off_knob;
            }
            ctx.draw_circle(knob, clip_rect, smooth_color(off_knob, on_knob, animation));

            if(!toggle->label_layout)
            {
                f32 label_offset = style_f32(ctx, node, Name("gui.switch.label_offset"), 56.0f);
                f32 font_size = style_f32(ctx, node, Name("gui.switch.font_size"), 16.0f);
                Float4U text_color = state.enabled ?
                    style_f32x4(ctx, node, Name("gui.switch.text_color"), Float4U(1.0f)) :
                    style_f32x4(ctx, node, Name("gui.switch.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
                ctx.draw_text(RectF(rect.offset_x + label_offset, rect.offset_y, max(rect.width - label_offset, 1.0f), rect.height),
                    clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
            }
        }

        static void draw_default_collapsing_header(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            Float4U background = state.hovered ?
                style_f32x4(ctx, node, Name("gui.collapsing_header.background_hovered"), Float4U(0.22f, 0.27f, 0.34f, 1.0f)) :
                style_f32x4(ctx, node, Name("gui.collapsing_header.background"), Float4U(0.16f, 0.19f, 0.24f, 1.0f));
            if(!state.enabled)
            {
                background = style_f32x4(ctx, node, Name("gui.collapsing_header.background_disabled"), disabled_alpha(ctx, node, background));
            }
            ctx.draw_rect(rect, clip_rect, background, style_f32(ctx, node, Name("gui.collapsing_header.radius"), 4.0f));
            f32 font_size = style_f32(ctx, node, Name("gui.collapsing_header.font_size"), 16.0f);
            Float4U text_color = state.enabled ?
                style_f32x4(ctx, node, Name("gui.collapsing_header.text_color"), Float4U(1.0f)) :
                style_f32x4(ctx, node, Name("gui.collapsing_header.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
            ctx.draw_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 8.0f, 1.0f), rect.height),
                clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
        }

        static void draw_default_tree_node(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const TreeNodeNode* tree = cast_node<TreeNodeNode>(node);
            if(!tree) return;
            bool node_open = false;
            DisclosureState* open_state = ctx.get_widget_state<DisclosureState>(node.id);
            if(open_state)
            {
                node_open = open_state->open;
            }
            else
            {
                node_open = !tree->leaf() && test_flags(tree->flags, TreeNodeFlag::default_open);
            }
            if(tree->selected || state.hovered || state.active)
            {
                Float4U active_color = style_f32x4(ctx, node, Name("gui.tree_node.background_active"), Float4U(0.20f, 0.36f, 0.62f, 1.0f));
                Float4U selected_color = style_f32x4(ctx, node, Name("gui.tree_node.background_selected"), Float4U(0.16f, 0.25f, 0.38f, 1.0f));
                Float4U hovered_color = style_f32x4(ctx, node, Name("gui.tree_node.background_hovered"), Float4U(0.18f, 0.24f, 0.32f, 1.0f));
                Float4U background = state.enabled ? (state.active ? active_color : (tree->selected ? selected_color : hovered_color)) :
                    style_f32x4(ctx, node, Name("gui.tree_node.background_disabled"), disabled_alpha(ctx, node, selected_color));
                ctx.draw_rect(rect, clip_rect, background,
                    style_f32(ctx, node, Name("gui.tree_node.radius"), 4.0f));
            }
            RectF arrow = tree->arrow_rect(rect);
            Float4U icon_color = tree->leaf() ?
                style_f32x4(ctx, node, Name("gui.tree_node.leaf_icon_color"), Float4U(0.58f, 0.65f, 0.74f, 1.0f)) :
                style_f32x4(ctx, node, Name("gui.tree_node.icon_color"), Float4U(1.0f));
            if(!state.enabled)
            {
                icon_color = style_f32x4(ctx, node, Name("gui.tree_node.icon_disabled"), Float4U(0.50f, 0.54f, 0.60f, 1.0f));
            }
            if(tree->leaf())
            {
                f32 dot = style_f32(ctx, node, Name("gui.tree_node.leaf_dot_size"), 5.0f);
                ctx.draw_circle(RectF(arrow.offset_x + (arrow.width - dot) * 0.5f, arrow.offset_y + (arrow.height - dot) * 0.5f, dot, dot),
                    clip_rect, icon_color);
            }
            else if(node_open)
            {
                f32 cx = arrow.offset_x + arrow.width * 0.5f;
                f32 cy = arrow.offset_y + arrow.height * 0.5f + 2.0f;
                ctx.draw_line(Float2U(cx - 5.0f, cy - 3.0f), Float2U(cx, cy + 3.0f), clip_rect, icon_color, 1.8f);
                ctx.draw_line(Float2U(cx, cy + 3.0f), Float2U(cx + 5.0f, cy - 3.0f), clip_rect, icon_color, 1.8f);
            }
            else
            {
                f32 cx = arrow.offset_x + arrow.width * 0.5f + 2.0f;
                f32 cy = arrow.offset_y + arrow.height * 0.5f;
                ctx.draw_line(Float2U(cx - 3.0f, cy - 5.0f), Float2U(cx + 3.0f, cy), clip_rect, icon_color, 1.8f);
                ctx.draw_line(Float2U(cx + 3.0f, cy), Float2U(cx - 3.0f, cy + 5.0f), clip_rect, icon_color, 1.8f);
            }
            f32 label_x = arrow.offset_x + arrow.width + 2.0f;
            f32 font_size = style_f32(ctx, node, Name("gui.tree_node.font_size"), 15.0f);
            Float4U text_color = state.enabled ?
                style_f32x4(ctx, node, Name("gui.tree_node.text_color"), Float4U(1.0f)) :
                style_f32x4(ctx, node, Name("gui.tree_node.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
            ctx.draw_text(RectF(label_x, rect.offset_y, max(rect.offset_x + rect.width - label_x - 6.0f, 1.0f), rect.height),
                clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
        }

        static void draw_default_radio_button(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const RadioButtonNode* radio = cast_node<RadioButtonNode>(node);
            if(!radio) return;
            f32 outer_size = style_f32(ctx, node, Name("gui.radio_button.outer_size"), 18.0f);
            RectF outer(rect.offset_x + 2.0f, rect.offset_y + max((rect.height - outer_size) * 0.5f, 0.0f), outer_size, outer_size);
            Float4U ring_color = state.hovered ?
                style_f32x4(ctx, node, Name("gui.radio_button.ring_hovered"), Float4U(0.38f, 0.43f, 0.50f, 1.0f)) :
                style_f32x4(ctx, node, Name("gui.radio_button.ring"), Float4U(0.27f, 0.31f, 0.37f, 1.0f));
            if(!state.enabled)
            {
                ring_color = style_f32x4(ctx, node, Name("gui.radio_button.ring_disabled"), Float4U(0.18f, 0.21f, 0.26f, 1.0f));
            }
            ctx.draw_circle(RectF(outer.offset_x - 1.0f, outer.offset_y - 1.0f, outer.width + 2.0f, outer.height + 2.0f), clip_rect, ring_color);
            ctx.draw_circle(outer, clip_rect, state.enabled ?
                style_f32x4(ctx, node, Name("gui.radio_button.background"), Float4U(0.10f, 0.12f, 0.15f, 1.0f)) :
                style_f32x4(ctx, node, Name("gui.radio_button.background_disabled"), Float4U(0.08f, 0.09f, 0.11f, 1.0f)));
            if(radio->selected_state())
            {
                f32 inner_size = style_f32(ctx, node, Name("gui.radio_button.inner_size"), 8.0f);
                ctx.draw_circle(RectF(outer.offset_x + (outer.width - inner_size) * 0.5f, outer.offset_y + (outer.height - inner_size) * 0.5f, inner_size, inner_size),
                    clip_rect, state.enabled ?
                        style_f32x4(ctx, node, Name("gui.radio_button.selected_color"), Float4U(0.34f, 0.58f, 0.92f, 1.0f)) :
                        style_f32x4(ctx, node, Name("gui.radio_button.selected_disabled"), Float4U(0.45f, 0.50f, 0.58f, 1.0f)));
            }
            if(!radio->label_layout)
            {
                f32 label_offset = style_f32(ctx, node, Name("gui.radio_button.label_offset"), 28.0f);
                f32 font_size = style_f32(ctx, node, Name("gui.radio_button.font_size"), 16.0f);
                Float4U text_color = state.enabled ?
                    style_f32x4(ctx, node, Name("gui.radio_button.text_color"), Float4U(1.0f)) :
                    style_f32x4(ctx, node, Name("gui.radio_button.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
                ctx.draw_text(RectF(rect.offset_x + label_offset, rect.offset_y, max(rect.width - label_offset, 1.0f), rect.height),
                    clip_rect, node.text.c_str(), font_size, text_color, TextAlignment::begin);
            }
        }

        static void draw_default_button_group(NodeRenderContext& ctx, const Node& node, const RectF& rect, const RectF& clip_rect,
            const NodeRenderState& state, void*)
        {
            const ButtonGroupNode* group = cast_node<ButtonGroupNode>(node);
            if(!group) return;
            u32 count = (u32)group->items.size();
            if(!count) return;
            i32 hover_item = state.enabled && state.hovered ? group->item_at(rect, state.pointer_position) : -1;
            i32 active_item = state.enabled && state.active ? group->item_at(rect, state.pointer_position) : -1;
            f32 radius = style_f32(ctx, node, Name("gui.button_group.radius"), min(5.0f, min(rect.width, rect.height) * 0.5f));
            RectF inner(rect.offset_x + 1.0f, rect.offset_y + 1.0f, max(rect.width - 2.0f, 1.0f), max(rect.height - 2.0f, 1.0f));
            f32 inner_radius = max(radius - 1.0f, 0.0f);
            Float4U border_color = style_f32x4(ctx, node, Name("gui.button_group.border"), Float4U(0.25f, 0.29f, 0.35f, 1.0f));
            Float4U bg_color = style_f32x4(ctx, node, Name("gui.button_group.background"), Float4U(0.07f, 0.08f, 0.10f, 1.0f));
            Float4U selected_color = style_f32x4(ctx, node, Name("gui.button_group.selected"), Float4U(0.16f, 0.24f, 0.38f, 1.0f));
            Float4U selected_hot_color = style_f32x4(ctx, node, Name("gui.button_group.selected_hot"), Float4U(0.20f, 0.33f, 0.54f, 1.0f));
            Float4U hover_color = style_f32x4(ctx, node, Name("gui.button_group.hover"), Float4U(0.14f, 0.17f, 0.22f, 1.0f));
            if(!state.enabled)
            {
                border_color = style_f32x4(ctx, node, Name("gui.button_group.border_disabled"), Float4U(0.18f, 0.21f, 0.26f, 1.0f));
                bg_color = style_f32x4(ctx, node, Name("gui.button_group.background_disabled"), Float4U(0.08f, 0.09f, 0.11f, 1.0f));
                selected_color = style_f32x4(ctx, node, Name("gui.button_group.selected_disabled"), Float4U(0.18f, 0.21f, 0.27f, 1.0f));
                selected_hot_color = selected_color;
                hover_color = bg_color;
            }

            ctx.draw_rect(rect, clip_rect, border_color, radius);
            ctx.draw_rect(inner, clip_rect, bg_color, inner_radius);

            f32 blend = clamp(state.delta_time * style_f32(ctx, node, Name("gui.button_group.animation_speed"), 14.0f), 0.0f, 1.0f);
            if(group->current_item)
            {
                f32 target = (f32)clamp(*group->current_item, 0, (i32)count - 1);
                f32 selection_animation = target;
                ButtonGroupAnimationState* animation_state = ctx.get_widget_state<ButtonGroupAnimationState>(node.id);
                if(animation_state && animation_state->selection_animation_initialized)
                {
                    selection_animation = animation_state->selection_animation;
                }
                selection_animation += (target - selection_animation) * blend;
                Ref<ButtonGroupAnimationState> next_animation_state = ctx.get_or_create_widget_state<ButtonGroupAnimationState>(node.id);
                next_animation_state->selection_animation = selection_animation;
                next_animation_state->selection_animation_initialized = true;
                f32 item_width = inner.width / (f32)count;
                RectF selection_rect(inner.offset_x + item_width * selection_animation, inner.offset_y, item_width, inner.height);
                f32 max_x = inner.offset_x + inner.width;
                if(selection_rect.offset_x + selection_rect.width > max_x)
                {
                    selection_rect.width = max(max_x - selection_rect.offset_x, 1.0f);
                }
                ctx.draw_rect(selection_rect, clip_rect, active_item == (i32)target ? selected_hot_color : selected_color, inner_radius);
            }
            else if(group->selected)
            {
                Vector<f32> animations;
                ButtonGroupAnimationState* animation_state = ctx.get_widget_state<ButtonGroupAnimationState>(node.id);
                if(animation_state)
                {
                    animations = animation_state->item_animations;
                }
                if(animations.size() != count)
                {
                    animations.assign(count, 0.0f);
                    for(u32 i = 0; i < count; ++i)
                    {
                        animations[i] = group->selected[i] ? 1.0f : 0.0f;
                    }
                }
                for(u32 i = 0; i < count; ++i)
                {
                    f32 target = group->selected[i] ? 1.0f : 0.0f;
                    animations[i] += (target - animations[i]) * blend;
                    f32 t = clamp(animations[i], 0.0f, 1.0f);
                    RectF button_rect = group->item_rect(inner, i);
                    Float4U base_color = (active_item == (i32)i || hover_item == (i32)i) ? hover_color : bg_color;
                    if(t > 0.001f || hover_item == (i32)i || active_item == (i32)i)
                    {
                        ctx.draw_rect_corners(button_rect, clip_rect, smooth_color(base_color, selected_color, t), inner_radius,
                            i == 0, i + 1 == count, i + 1 == count, i == 0);
                    }
                }
                Ref<ButtonGroupAnimationState> next_animation_state = ctx.get_or_create_widget_state<ButtonGroupAnimationState>(node.id);
                next_animation_state->item_animations = move(animations);
            }
            if(group->current_item && hover_item >= 0 && hover_item != *group->current_item)
            {
                RectF button_rect = group->item_rect(inner, (u32)hover_item);
                ctx.draw_rect_corners(button_rect, clip_rect, hover_color, inner_radius,
                    hover_item == 0, (u32)hover_item + 1 == count, (u32)hover_item + 1 == count, hover_item == 0);
            }
            Float4U separator_color = style_f32x4(ctx, node, Name("gui.button_group.separator"), Float4U(0.20f, 0.23f, 0.28f, 0.90f));
            if(!state.enabled)
            {
                separator_color = style_f32x4(ctx, node, Name("gui.button_group.separator_disabled"), Float4U(0.14f, 0.16f, 0.19f, 0.90f));
            }
            for(u32 i = 1; i < count; ++i)
            {
                f32 x = rect.offset_x + rect.width * ((f32)i / (f32)count);
                ctx.draw_line(Float2U(x, rect.offset_y + 2.0f), Float2U(x, rect.offset_y + max(rect.height - 2.0f, 2.0f)),
                    clip_rect, separator_color, 1.0f);
            }
            f32 font_size = style_f32(ctx, node, Name("gui.button_group.font_size"), 15.0f);
            Float4U selected_text_color = style_f32x4(ctx, node, Name("gui.button_group.text_selected"), Float4U(1.0f));
            Float4U text_color = style_f32x4(ctx, node, Name("gui.button_group.text"), Float4U(0.58f, 0.63f, 0.70f, 1.0f));
            if(!state.enabled)
            {
                selected_text_color = style_f32x4(ctx, node, Name("gui.button_group.text_disabled"), Float4U(0.55f, 0.59f, 0.65f, 1.0f));
                text_color = selected_text_color;
            }
            for(u32 i = 0; i < count; ++i)
            {
                RectF button_rect = group->item_rect(inner, i);
                bool item_selected = group->current_item ? *group->current_item == (i32)i : (group->selected && group->selected[i]);
                ctx.draw_text(RectF(button_rect.offset_x + 8.0f, button_rect.offset_y, max(button_rect.width - 16.0f, 1.0f), button_rect.height),
                    clip_rect, group->items[i].c_str(), font_size, item_selected ? selected_text_color : text_color, TextAlignment::center);
            }
        }

        RenderProxyDesc default_button_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_F32X4("gui.button.background", Float4U(0.18f, 0.28f, 0.45f, 1.0f), "Button"),
                LUNA_GUI_STYLE_F32X4("gui.button.background_hovered", Float4U(0.26f, 0.43f, 0.72f, 1.0f), "Button"),
                LUNA_GUI_STYLE_F32X4("gui.button.background_active", Float4U(0.20f, 0.36f, 0.62f, 1.0f), "Button"),
                LUNA_GUI_STYLE_F32X4("gui.button.background_disabled", Float4U(0.13f, 0.16f, 0.20f, 1.0f), "Button"),
                LUNA_GUI_STYLE_F32("gui.button.animation_speed", 14.0f, "Button"),
                LUNA_GUI_STYLE_F32("gui.button.radius", 5.0f, "Button")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_button;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_button_label_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.button.font_size", 16.0f, "Button"),
                LUNA_GUI_STYLE_F32X4("gui.button.text_color", Float4U(1.0f), "Button"),
                LUNA_GUI_STYLE_F32X4("gui.button.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "Button")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_button_label;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_progress_bar_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.progress_bar.border_size", 1.0f, "ProgressBar"),
                LUNA_GUI_STYLE_F32("gui.progress_bar.radius", 5.0f, "ProgressBar"),
                LUNA_GUI_STYLE_F32X4("gui.progress_bar.border", Float4U(0.25f, 0.29f, 0.35f, 1.0f), "ProgressBar"),
                LUNA_GUI_STYLE_F32X4("gui.progress_bar.background", Float4U(0.07f, 0.08f, 0.10f, 1.0f), "ProgressBar"),
                LUNA_GUI_STYLE_F32X4("gui.progress_bar.fill", Float4U(0.20f, 0.36f, 0.62f, 1.0f), "ProgressBar"),
                LUNA_GUI_STYLE_F32("gui.progress_bar.font_size", 14.0f, "ProgressBar"),
                LUNA_GUI_STYLE_F32X4("gui.progress_bar.text_color", Float4U(1.0f), "ProgressBar")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_progress_bar;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_text_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.text.font_size", 16.0f, "Text"),
                LUNA_GUI_STYLE_F32X4("gui.text.color", Float4U(1.0f), "Text"),
                LUNA_GUI_STYLE_F32X4("gui.text.disabled", Float4U(0.52f, 0.52f, 0.52f, 0.52f), "Text"),
                LUNA_GUI_STYLE_F32("gui.disabled_alpha", 0.52f, "Common")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_text;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_selectable_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32X4("gui.selectable.background_active", Float4U(0.20f, 0.36f, 0.62f, 1.0f), "Selectable"),
                LUNA_GUI_STYLE_F32X4("gui.selectable.background_selected", Float4U(0.16f, 0.25f, 0.38f, 1.0f), "Selectable"),
                LUNA_GUI_STYLE_F32X4("gui.selectable.background_hovered", Float4U(0.20f, 0.30f, 0.44f, 1.0f), "Selectable"),
                LUNA_GUI_STYLE_F32("gui.selectable.radius", 4.0f, "Selectable"),
                LUNA_GUI_STYLE_F32("gui.selectable.font_size", 15.0f, "Selectable"),
                LUNA_GUI_STYLE_F32X4("gui.selectable.text_color", Float4U(1.0f), "Selectable"),
                LUNA_GUI_STYLE_F32X4("gui.selectable.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "Selectable"),
                LUNA_GUI_STYLE_F32("gui.disabled_alpha", 0.52f, "Common")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_selectable;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_checkbox_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.checkbox.box_size", 18.0f, "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.border_disabled", Float4U(0.18f, 0.21f, 0.26f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.border_hovered", Float4U(0.34f, 0.39f, 0.46f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.border", Float4U(0.25f, 0.29f, 0.35f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.background_checked", Float4U(0.22f, 0.55f, 0.32f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.background", Float4U(0.18f, 0.20f, 0.23f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.background_checked_disabled", Float4U(0.1144f, 0.286f, 0.1664f, 0.52f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.background_disabled", Float4U(0.12f, 0.14f, 0.17f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32("gui.checkbox.radius", 3.0f, "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.check_color", Float4U(1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.check_disabled", Float4U(0.70f, 0.74f, 0.80f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32("gui.checkbox.check_width", 2.4f, "Checkbox"),
                LUNA_GUI_STYLE_F32("gui.checkbox.label_offset", 28.0f, "Checkbox"),
                LUNA_GUI_STYLE_F32("gui.checkbox.font_size", 16.0f, "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.text_color", Float4U(1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32X4("gui.checkbox.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "Checkbox"),
                LUNA_GUI_STYLE_F32("gui.disabled_alpha", 0.52f, "Common")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_checkbox;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_toggle_switch_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.switch.animation_speed", 14.0f, "Switch"),
                LUNA_GUI_STYLE_F32X2("gui.switch.track_size", Float2U(44.0f, 22.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.track_disabled", Float4U(0.11f, 0.12f, 0.14f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.off_track_hovered", Float4U(0.18f, 0.20f, 0.23f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.off_track", Float4U(0.12f, 0.14f, 0.16f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.track_checked_disabled", Float4U(0.17f, 0.23f, 0.20f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.on_track_hovered", Float4U(0.25f, 0.62f, 0.38f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.on_track", Float4U(0.20f, 0.55f, 0.32f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32("gui.switch.knob_size", 18.0f, "Switch"),
                LUNA_GUI_STYLE_F32("gui.switch.knob_margin", 2.0f, "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.off_knob", Float4U(0.78f, 0.80f, 0.84f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.on_knob", Float4U(1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.knob_disabled", Float4U(0.45f, 0.48f, 0.54f, 1.0f), "Switch"),
                LUNA_GUI_STYLE_F32("gui.switch.label_offset", 56.0f, "Switch"),
                LUNA_GUI_STYLE_F32("gui.switch.font_size", 16.0f, "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.text_color", Float4U(1.0f), "Switch"),
                LUNA_GUI_STYLE_F32X4("gui.switch.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "Switch")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_toggle_switch;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_collapsing_header_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32X4("gui.collapsing_header.background_hovered", Float4U(0.22f, 0.27f, 0.34f, 1.0f), "CollapsingHeader"),
                LUNA_GUI_STYLE_F32X4("gui.collapsing_header.background", Float4U(0.16f, 0.19f, 0.24f, 1.0f), "CollapsingHeader"),
                LUNA_GUI_STYLE_F32X4("gui.collapsing_header.background_disabled", Float4U(0.0832f, 0.0988f, 0.1248f, 0.52f), "CollapsingHeader"),
                LUNA_GUI_STYLE_F32("gui.collapsing_header.radius", 4.0f, "CollapsingHeader"),
                LUNA_GUI_STYLE_F32("gui.collapsing_header.font_size", 16.0f, "CollapsingHeader"),
                LUNA_GUI_STYLE_F32X4("gui.collapsing_header.text_color", Float4U(1.0f), "CollapsingHeader"),
                LUNA_GUI_STYLE_F32X4("gui.collapsing_header.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "CollapsingHeader"),
                LUNA_GUI_STYLE_F32("gui.disabled_alpha", 0.52f, "Common")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_collapsing_header;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_tree_node_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.background_active", Float4U(0.20f, 0.36f, 0.62f, 1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.background_selected", Float4U(0.16f, 0.25f, 0.38f, 1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.background_hovered", Float4U(0.18f, 0.24f, 0.32f, 1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.background_disabled", Float4U(0.0832f, 0.13f, 0.1976f, 0.52f), "TreeNode"),
                LUNA_GUI_STYLE_F32("gui.tree_node.radius", 4.0f, "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.leaf_icon_color", Float4U(0.58f, 0.65f, 0.74f, 1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.icon_color", Float4U(1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.icon_disabled", Float4U(0.50f, 0.54f, 0.60f, 1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32("gui.tree_node.leaf_dot_size", 5.0f, "TreeNode"),
                LUNA_GUI_STYLE_F32("gui.tree_node.font_size", 15.0f, "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.text_color", Float4U(1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32X4("gui.tree_node.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "TreeNode"),
                LUNA_GUI_STYLE_F32("gui.disabled_alpha", 0.52f, "Common")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_tree_node;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_radio_button_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.radio_button.outer_size", 18.0f, "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.ring_hovered", Float4U(0.38f, 0.43f, 0.50f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.ring", Float4U(0.27f, 0.31f, 0.37f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.ring_disabled", Float4U(0.18f, 0.21f, 0.26f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.background", Float4U(0.10f, 0.12f, 0.15f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.background_disabled", Float4U(0.08f, 0.09f, 0.11f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32("gui.radio_button.inner_size", 8.0f, "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.selected_color", Float4U(0.34f, 0.58f, 0.92f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.selected_disabled", Float4U(0.45f, 0.50f, 0.58f, 1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32("gui.radio_button.label_offset", 28.0f, "RadioButton"),
                LUNA_GUI_STYLE_F32("gui.radio_button.font_size", 16.0f, "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.text_color", Float4U(1.0f), "RadioButton"),
                LUNA_GUI_STYLE_F32X4("gui.radio_button.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "RadioButton")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_radio_button;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

        RenderProxyDesc default_button_group_render_proxy()
        {
            static StyleEntryDesc entries[] = {
                LUNA_GUI_STYLE_NAME("gui.font", "", "Text"),
                LUNA_GUI_STYLE_F32("gui.button_group.radius", 5.0f, "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.border", Float4U(0.25f, 0.29f, 0.35f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.background", Float4U(0.07f, 0.08f, 0.10f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.selected", Float4U(0.16f, 0.24f, 0.38f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.selected_hot", Float4U(0.20f, 0.33f, 0.54f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.hover", Float4U(0.14f, 0.17f, 0.22f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.border_disabled", Float4U(0.18f, 0.21f, 0.26f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.background_disabled", Float4U(0.08f, 0.09f, 0.11f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.selected_disabled", Float4U(0.18f, 0.21f, 0.27f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32("gui.button_group.animation_speed", 14.0f, "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.separator", Float4U(0.20f, 0.23f, 0.28f, 0.90f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.separator_disabled", Float4U(0.14f, 0.16f, 0.19f, 0.90f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32("gui.button_group.font_size", 15.0f, "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.text_selected", Float4U(1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.text", Float4U(0.58f, 0.63f, 0.70f, 1.0f), "ButtonGroup"),
                LUNA_GUI_STYLE_F32X4("gui.button_group.text_disabled", Float4U(0.55f, 0.59f, 0.65f, 1.0f), "ButtonGroup")
            };
            RenderProxyDesc desc;
            desc.draw = draw_default_button_group;
            desc.style_entries = entries;
            desc.num_style_entries = sizeof(entries) / sizeof(entries[0]);
            return desc;
        }

#undef LUNA_GUI_STYLE_F32
#undef LUNA_GUI_STYLE_F32X2
#undef LUNA_GUI_STYLE_F32X4
#undef LUNA_GUI_STYLE_NAME
    }
}
