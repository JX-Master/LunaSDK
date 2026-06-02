/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/BasicNodes.hpp"
#include "../../State.hpp"
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        Guid ButtonNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ButtonNode::clone() const
        {
            return new_object<ButtonNode>(*this);
        }

        LayoutMetrics ButtonNode::measure() const
        {
            f32 text_width = (f32)text.size() * 16.0f * 0.52f;
            return fixed_height_metrics(72.0f, max(text_width + 24.0f, 72.0f), 30.0f);
        }

        void ButtonNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            Float4U color = state.active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                (state.hovered ? Float4U(0.26f, 0.43f, 0.72f, 1.0f) : Float4U(0.18f, 0.28f, 0.45f, 1.0f));
            ctx.draw_rect(rect, clip_rect, color, 5.0f);
            ctx.draw_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height),
                clip_rect, text.c_str(), 16.0f, Float4U(1.0f), TextAlignment::center);
        }

        Guid TextNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> TextNode::clone() const
        {
            return new_object<TextNode>(*this);
        }

        LayoutMetrics TextNode::measure() const
        {
            f32 text_width = (f32)text.size() * font_size * 0.52f;
            return fixed_height_metrics(min(max(text_width, 1.0f), 32.0f), max(text_width, 1.0f), font_size + 4.0f);
        }

        LayoutMetrics TextNode::measure(NodeMeasureContext& ctx) const
        {
            f32 max_width = F32_MAX;
            const Node* parent_node = ctx.parent();
            const TooltipNode* tooltip = parent_node ? tooltip_node(*parent_node) : nullptr;
            if(tooltip)
            {
                f32 tooltip_width = parent_node->requested_size.width > 0.0f ? parent_node->requested_size.width : tooltip->desc.max_width;
                if(tooltip_width <= 0.0f)
                {
                    tooltip_width = ctx.surface_size().x;
                }
                tooltip_width = min(tooltip_width, max(ctx.surface_size().x, 1.0f));
                max_width = max(tooltip_width - parent_node->layout_desc.padding.left - parent_node->layout_desc.padding.right, 1.0f);
            }
            return ctx.measure_text(text.c_str(), text.size(), font_size, max_width);
        }

        void TextNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_text(rect, clip_rect, text.c_str(), font_size, color, TextAlignment::begin);
        }

        SelectableNode::SelectableNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid SelectableNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> SelectableNode::clone() const
        {
            return new_object<SelectableNode>(*this);
        }

        NodeLayoutFlow SelectableNode::layout_flow() const
        {
            return NodeLayoutFlow::horizontal;
        }

        bool SelectableNode::default_interactive() const
        {
            return true;
        }

        bool SelectableNode::uses_node_measure() const
        {
            return !label_layout;
        }

        void SelectableNode::apply_container_defaults(LayoutDesc& desc) const
        {
            if(!label_layout) return;
            desc.padding = EdgeInsets::xy(8.0f, 3.0f);
            desc.gap = 0.0f;
            desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        }

        LayoutMetrics SelectableNode::measure() const
        {
            f32 text_width = (f32)text.size() * 16.0f * 0.52f;
            LayoutMetrics metrics;
            metrics.min_size = Float2U(72.0f, 26.0f);
            metrics.preferred_size = Float2U(max(text_width + 24.0f, 72.0f), 26.0f);
            metrics.max_size = Float2U(F32_MAX, 26.0f);
            return metrics;
        }

        void SelectableNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            if(selected || state.hovered || state.active)
            {
                Float4U color = state.active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                    (state.hovered ? Float4U(0.20f, 0.30f, 0.44f, 1.0f) : Float4U(0.16f, 0.25f, 0.38f, 1.0f));
                ctx.draw_rect(rect, clip_rect, color, 4.0f);
            }
            if(!label_layout)
            {
                ctx.draw_text(RectF(rect.offset_x + 8.0f, rect.offset_y, max(rect.width - 16.0f, 1.0f), rect.height),
                    clip_rect, text.c_str(), 15.0f, Float4U(1.0f), TextAlignment::begin);
            }
        }

        Guid CheckboxNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> CheckboxNode::clone() const
        {
            return new_object<CheckboxNode>(*this);
        }

        NodeLayoutFlow CheckboxNode::layout_flow() const
        {
            return NodeLayoutFlow::horizontal;
        }

        bool CheckboxNode::default_interactive() const
        {
            return true;
        }

        bool CheckboxNode::uses_node_measure() const
        {
            return !label_layout;
        }

        void CheckboxNode::apply_container_defaults(LayoutDesc& desc) const
        {
            if(!label_layout) return;
            desc.padding.left = 28.0f;
            desc.padding.top = 3.0f;
            desc.padding.bottom = 3.0f;
            desc.gap = 0.0f;
            desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        }

        LayoutMetrics CheckboxNode::measure() const
        {
            LayoutMetrics metrics;
            metrics.min_size = Float2U(26.0f, 26.0f);
            metrics.preferred_size = Float2U(26.0f, 26.0f);
            metrics.max_size = Float2U(26.0f, 26.0f);
            return metrics;
        }

        void CheckboxNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            RectF box(rect.offset_x + 2.0f, rect.offset_y + max((rect.height - 18.0f) * 0.5f, 0.0f), 18.0f, 18.0f);
            bool checked = value && *value;
            ctx.draw_rect(RectF(box.offset_x - 1.0f, box.offset_y - 1.0f, box.width + 2.0f, box.height + 2.0f),
                clip_rect, state.hovered ? Float4U(0.34f, 0.39f, 0.46f, 1.0f) : Float4U(0.25f, 0.29f, 0.35f, 1.0f), 4.0f);
            ctx.draw_rect(box, clip_rect, checked ? Float4U(0.22f, 0.55f, 0.32f, 1.0f) : Float4U(0.18f, 0.20f, 0.23f, 1.0f), 3.0f);
            if(checked)
            {
                ctx.draw_line(Float2U(box.offset_x + 4.0f, box.offset_y + 9.5f),
                    Float2U(box.offset_x + 7.5f, box.offset_y + 13.0f), clip_rect, Float4U(1.0f), 2.4f);
                ctx.draw_line(Float2U(box.offset_x + 7.5f, box.offset_y + 13.0f),
                    Float2U(box.offset_x + 14.5f, box.offset_y + 5.5f), clip_rect, Float4U(1.0f), 2.4f);
            }
            if(!label_layout)
            {
                ctx.draw_text(RectF(rect.offset_x + 28.0f, rect.offset_y, max(rect.width - 28.0f, 1.0f), rect.height),
                    clip_rect, text.c_str(), 16.0f, Float4U(1.0f), TextAlignment::begin);
            }
        }

        void CheckboxNode::on_click(NodeInputContext& ctx)
        {
            if(!value) return;
            *value = !*value;
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        Guid ToggleSwitchNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ToggleSwitchNode::clone() const
        {
            return new_object<ToggleSwitchNode>(*this);
        }

        NodeLayoutFlow ToggleSwitchNode::layout_flow() const
        {
            return NodeLayoutFlow::horizontal;
        }

        bool ToggleSwitchNode::default_interactive() const
        {
            return true;
        }

        bool ToggleSwitchNode::uses_node_measure() const
        {
            return !label_layout;
        }

        void ToggleSwitchNode::apply_container_defaults(LayoutDesc& desc) const
        {
            if(!label_layout) return;
            desc.padding.left = 56.0f;
            desc.padding.top = 4.0f;
            desc.padding.bottom = 4.0f;
            desc.gap = 0.0f;
            desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        }

        LayoutMetrics ToggleSwitchNode::measure() const
        {
            LayoutMetrics metrics;
            metrics.min_size = Float2U(46.0f, 28.0f);
            metrics.preferred_size = Float2U(46.0f, 28.0f);
            metrics.max_size = Float2U(46.0f, 28.0f);
            return metrics;
        }

        void ToggleSwitchNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            bool checked = value && *value;
            f32 target = checked ? 1.0f : 0.0f;
            f32 animation = target;
            SwitchAnimationState* switch_state = ctx.get_widget_state<SwitchAnimationState>(id);
            if(switch_state && switch_state->initialized)
            {
                animation = switch_state->animation;
            }
            f32 blend = clamp(state.delta_time * 14.0f, 0.0f, 1.0f);
            animation += (target - animation) * blend;
            animation = clamp(animation, 0.0f, 1.0f);
            Ref<SwitchAnimationState> next_state = ctx.get_or_create_widget_state<SwitchAnimationState>(id);
            next_state->animation = animation;
            next_state->initialized = true;

            RectF track(rect.offset_x + 2.0f, rect.offset_y + max((rect.height - 22.0f) * 0.5f, 0.0f), 44.0f, 22.0f);
            Float4U off_track = state.hovered ? Float4U(0.18f, 0.20f, 0.23f, 1.0f) : Float4U(0.12f, 0.14f, 0.16f, 1.0f);
            Float4U on_track = state.hovered ? Float4U(0.25f, 0.62f, 0.38f, 1.0f) : Float4U(0.20f, 0.55f, 0.32f, 1.0f);
            ctx.draw_rect(track, clip_rect, smooth_color(off_track, on_track, animation), track.height * 0.5f);

            f32 knob_size = 18.0f;
            f32 knob_x = track.offset_x + 2.0f + (track.width - knob_size - 4.0f) * animation;
            RectF knob(knob_x, track.offset_y + 2.0f, knob_size, knob_size);
            ctx.draw_circle(knob, clip_rect, smooth_color(Float4U(0.78f, 0.80f, 0.84f, 1.0f), Float4U(1.0f), animation));

            if(!label_layout)
            {
                ctx.draw_text(RectF(rect.offset_x + 56.0f, rect.offset_y, max(rect.width - 56.0f, 1.0f), rect.height),
                    clip_rect, text.c_str(), 16.0f, Float4U(1.0f), TextAlignment::begin);
            }
        }

        void ToggleSwitchNode::on_click(NodeInputContext& ctx)
        {
            if(!value) return;
            *value = !*value;
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        Guid CollapsingHeaderNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> CollapsingHeaderNode::clone() const
        {
            return new_object<CollapsingHeaderNode>(*this);
        }

        bool CollapsingHeaderNode::open(NodeInputContext& ctx) const
        {
            DisclosureState* state = ctx.get_widget_state<DisclosureState>(id);
            return state ? state->open : true;
        }

        LayoutMetrics CollapsingHeaderNode::measure() const
        {
            f32 text_width = (f32)text.size() * 16.0f * 0.52f;
            LayoutMetrics metrics;
            metrics.min_size = Float2U(120.0f, 30.0f);
            metrics.preferred_size = Float2U(max(text_width + 32.0f, 120.0f), 30.0f);
            metrics.max_size = Float2U(F32_MAX, 30.0f);
            return metrics;
        }

        void CollapsingHeaderNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, state.hovered ? Float4U(0.22f, 0.27f, 0.34f, 1.0f) : Float4U(0.16f, 0.19f, 0.24f, 1.0f), 4.0f);
            ctx.draw_text(RectF(rect.offset_x + 8.0f, rect.offset_y, rect.width - 8.0f, rect.height),
                clip_rect, text.c_str(), 16.0f, Float4U(1.0f), TextAlignment::begin);
        }

        void CollapsingHeaderNode::update_state(NodeInputContext& ctx) const
        {
            Ref<DisclosureState> state = ctx.get_or_create_widget_state<DisclosureState>(id);
            state->open = open(ctx);
            ctx.set_state(Name("gui.open"), Any(state->open));
        }

        void CollapsingHeaderNode::on_click(NodeInputContext& ctx)
        {
            bool next_open = !open(ctx);
            ctx.get_or_create_widget_state<DisclosureState>(id)->open = next_open;
            ctx.set_state(Name("gui.open"), Any(next_open));
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        TreeNodeNode::TreeNodeNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid TreeNodeNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> TreeNodeNode::clone() const
        {
            return new_object<TreeNodeNode>(*this);
        }

        bool TreeNodeNode::leaf() const
        {
            return test_flags(flags, TreeNodeFlag::leaf);
        }

        bool TreeNodeNode::open(NodeInputContext& ctx) const
        {
            DisclosureState* state = ctx.get_widget_state<DisclosureState>(id);
            return state ? state->open : (!leaf() && test_flags(flags, TreeNodeFlag::default_open));
        }

        RectF TreeNodeNode::arrow_rect(const RectF& rect) const
        {
            f32 x = rect.offset_x + 4.0f + 18.0f * (f32)indent_depth;
            f32 y = rect.offset_y + max((rect.height - 18.0f) * 0.5f, 0.0f);
            return RectF(x, y, 18.0f, 18.0f);
        }

        LayoutMetrics TreeNodeNode::measure() const
        {
            f32 indent = 18.0f * (f32)indent_depth;
            f32 text_width = (f32)text.size() * 16.0f * 0.52f;
            f32 width = max(text_width + indent + 34.0f, 80.0f);
            LayoutMetrics metrics;
            metrics.min_size = Float2U(min(width, 80.0f), 26.0f);
            metrics.preferred_size = Float2U(width, 26.0f);
            metrics.max_size = Float2U(F32_MAX, 26.0f);
            return metrics;
        }

        void TreeNodeNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            bool node_open = false;
            DisclosureState* open_state = ctx.get_widget_state<DisclosureState>(id);
            if(open_state)
            {
                node_open = open_state->open;
            }
            else
            {
                node_open = !leaf() && test_flags(flags, TreeNodeFlag::default_open);
            }
            if(selected || state.hovered || state.active)
            {
                ctx.draw_rect(rect, clip_rect, state.active ? Float4U(0.20f, 0.36f, 0.62f, 1.0f) :
                    (selected ? Float4U(0.16f, 0.25f, 0.38f, 1.0f) : Float4U(0.18f, 0.24f, 0.32f, 1.0f)), 4.0f);
            }
            RectF arrow = arrow_rect(rect);
            Float4U icon_color = leaf() ? Float4U(0.58f, 0.65f, 0.74f, 1.0f) : Float4U(1.0f);
            if(leaf())
            {
                f32 dot = 5.0f;
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
            ctx.draw_text(RectF(label_x, rect.offset_y, max(rect.offset_x + rect.width - label_x - 6.0f, 1.0f), rect.height),
                clip_rect, text.c_str(), 15.0f, Float4U(1.0f), TextAlignment::begin);
        }

        void TreeNodeNode::update_state(NodeInputContext& ctx) const
        {
            bool node_open = open(ctx);
            ctx.get_or_create_widget_state<DisclosureState>(id)->open = node_open;
            ctx.set_state(Name("gui.open"), Any(node_open));
        }

        void TreeNodeNode::on_click(NodeInputContext& ctx)
        {
            if(leaf()) return;
            if(test_flags(flags, TreeNodeFlag::open_on_arrow))
            {
                RectF arrow = arrow_rect(ctx.rect());
                Float2U pos = ctx.pointer_position();
                if(pos.x < arrow.offset_x || pos.x > arrow.offset_x + arrow.width ||
                    pos.y < arrow.offset_y || pos.y > arrow.offset_y + arrow.height)
                {
                    return;
                }
            }
            bool next_open = !open(ctx);
            ctx.get_or_create_widget_state<DisclosureState>(id)->open = next_open;
            ctx.set_state(Name("gui.open"), Any(next_open));
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        Guid RadioButtonNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> RadioButtonNode::clone() const
        {
            return new_object<RadioButtonNode>(*this);
        }

        bool RadioButtonNode::selected_state() const
        {
            if(i32_value) return *i32_value == item_value;
            if(value) return *value;
            return selected;
        }

        NodeLayoutFlow RadioButtonNode::layout_flow() const
        {
            return NodeLayoutFlow::horizontal;
        }

        bool RadioButtonNode::default_interactive() const
        {
            return true;
        }

        bool RadioButtonNode::uses_node_measure() const
        {
            return !label_layout;
        }

        void RadioButtonNode::apply_container_defaults(LayoutDesc& desc) const
        {
            if(!label_layout) return;
            desc.padding.left = 28.0f;
            desc.padding.top = 3.0f;
            desc.padding.bottom = 3.0f;
            desc.gap = 0.0f;
            desc.cross_axis_alignment = LayoutCrossAxisAlignment::stretch;
        }

        LayoutMetrics RadioButtonNode::measure() const
        {
            LayoutMetrics metrics;
            metrics.min_size = Float2U(26.0f, 26.0f);
            metrics.preferred_size = Float2U(26.0f, 26.0f);
            metrics.max_size = Float2U(26.0f, 26.0f);
            return metrics;
        }

        void RadioButtonNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            RectF outer(rect.offset_x + 2.0f, rect.offset_y + max((rect.height - 18.0f) * 0.5f, 0.0f), 18.0f, 18.0f);
            ctx.draw_circle(RectF(outer.offset_x - 1.0f, outer.offset_y - 1.0f, outer.width + 2.0f, outer.height + 2.0f),
                clip_rect, state.hovered ? Float4U(0.38f, 0.43f, 0.50f, 1.0f) : Float4U(0.27f, 0.31f, 0.37f, 1.0f));
            ctx.draw_circle(outer, clip_rect, Float4U(0.10f, 0.12f, 0.15f, 1.0f));
            if(selected_state())
            {
                ctx.draw_circle(RectF(outer.offset_x + 5.0f, outer.offset_y + 5.0f, 8.0f, 8.0f),
                    clip_rect, Float4U(0.34f, 0.58f, 0.92f, 1.0f));
            }
            if(!label_layout)
            {
                ctx.draw_text(RectF(rect.offset_x + 28.0f, rect.offset_y, max(rect.width - 28.0f, 1.0f), rect.height),
                    clip_rect, text.c_str(), 16.0f, Float4U(1.0f), TextAlignment::begin);
            }
        }

        void RadioButtonNode::on_click(NodeInputContext& ctx)
        {
            if(i32_value)
            {
                if(*i32_value != item_value)
                {
                    *i32_value = item_value;
                    selected = true;
                    ctx.set_state(Name("gui.value_changed"), Any(true));
                }
            }
            else if(value && !*value)
            {
                *value = true;
                selected = true;
                ctx.set_state(Name("gui.value_changed"), Any(true));
            }
        }

    }
}
