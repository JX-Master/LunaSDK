/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "BasicNodes.hpp"
#include "../../State.hpp"
#include "../GUI.hpp"
#include "../RenderProxies/BasicRenderProxies.hpp"

namespace Luna
{
    namespace GUI
    {
        ButtonNode::ButtonNode()
        {
            render_proxy = default_button_render_proxy();
        }

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

        ProgressBarNode::ProgressBarNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_progress_bar_render_proxy();
        }

        Guid ProgressBarNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ProgressBarNode::clone() const
        {
            return new_object<ProgressBarNode>(*this);
        }

        LayoutMetrics ProgressBarNode::measure() const
        {
            return fixed_height_metrics(80.0f, 160.0f, 24.0f);
        }

        TextNode::TextNode()
        {
            render_proxy = default_text_render_proxy();
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

        SelectableNode::SelectableNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_selectable_render_proxy();
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

        CheckboxNode::CheckboxNode()
        {
            render_proxy = default_checkbox_render_proxy();
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

        void CheckboxNode::on_click(NodeInputContext& ctx)
        {
            if(!value) return;
            *value = !*value;
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        ToggleSwitchNode::ToggleSwitchNode()
        {
            render_proxy = default_toggle_switch_render_proxy();
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

        void ToggleSwitchNode::on_click(NodeInputContext& ctx)
        {
            if(!value) return;
            *value = !*value;
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        CollapsingHeaderNode::CollapsingHeaderNode()
        {
            render_proxy = default_collapsing_header_render_proxy();
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
            render_proxy = default_tree_node_render_proxy();
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

        RadioButtonNode::RadioButtonNode()
        {
            render_proxy = default_radio_button_render_proxy();
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
