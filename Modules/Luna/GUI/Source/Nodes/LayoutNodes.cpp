/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "LayoutNodes.hpp"

namespace Luna
{
    namespace GUI
    {
        Guid RootNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> RootNode::clone() const
        {
            return new_object<RootNode>(*this);
        }

        NodeLayerRole RootNode::layer_role() const
        {
            return NodeLayerRole::root;
        }

        bool RootNode::uses_node_measure() const
        {
            return false;
        }

        Guid HLayoutNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> HLayoutNode::clone() const
        {
            return new_object<HLayoutNode>(*this);
        }

        NodeLayoutFlow HLayoutNode::layout_flow() const
        {
            return NodeLayoutFlow::horizontal;
        }

        bool HLayoutNode::uses_node_measure() const
        {
            return false;
        }

        Guid VLayoutNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> VLayoutNode::clone() const
        {
            return new_object<VLayoutNode>(*this);
        }

        bool VLayoutNode::uses_node_measure() const
        {
            return false;
        }

        Guid ScrollViewNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> ScrollViewNode::clone() const
        {
            return new_object<ScrollViewNode>(*this);
        }

        NodeLayoutBehavior ScrollViewNode::layout_behavior() const
        {
            return NodeLayoutBehavior::scroll;
        }

        bool ScrollViewNode::default_interactive() const
        {
            return true;
        }

        bool ScrollViewNode::uses_node_measure() const
        {
            return false;
        }

        void ScrollViewNode::apply_container_defaults(LayoutDesc& desc) const
        {
            desc.padding = EdgeInsets::all(8.0f);
        }

        void ScrollViewNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.10f, 0.12f, 0.14f, 0.92f), 6.0f);
        }

        Guid WindowNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> WindowNode::clone() const
        {
            return new_object<WindowNode>(*this);
        }

        bool WindowNode::uses_node_measure() const
        {
            return false;
        }

        void WindowNode::apply_container_defaults(LayoutDesc& desc) const
        {
            desc.padding = EdgeInsets::all(8.0f);
        }

        f32 WindowNode::title_bar_height()
        {
            return 30.0f;
        }

        RectF WindowNode::close_rect(const RectF& rect)
        {
            return RectF(rect.offset_x + max(rect.width - 26.0f, 0.0f), rect.offset_y + 5.0f, 20.0f, 20.0f);
        }

        void WindowNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.10f, 0.12f, 0.14f, 0.92f), 6.0f);
            if(!open) return;
            RectF title_rect(rect.offset_x, rect.offset_y, rect.width, title_bar_height());
            ctx.draw_rect(title_rect, clip_rect, Float4U(0.13f, 0.17f, 0.22f, 1.0f), 6.0f);
            ctx.draw_text(RectF(rect.offset_x + 10.0f, rect.offset_y, max(rect.width - 46.0f, 1.0f), title_bar_height()),
                clip_rect, text.c_str(), 15.0f, Float4U(1.0f), TextAlignment::begin);
            RectF close = close_rect(rect);
            bool close_hovered = state.pointer_position.x >= close.offset_x && state.pointer_position.x <= close.offset_x + close.width &&
                state.pointer_position.y >= close.offset_y && state.pointer_position.y <= close.offset_y + close.height;
            ctx.draw_rect(close, clip_rect, close_hovered ? Float4U(0.55f, 0.18f, 0.18f, 1.0f) : Float4U(0.23f, 0.27f, 0.33f, 1.0f), 4.0f);
            ctx.draw_text(close, clip_rect, "X", 14.0f, Float4U(1.0f), TextAlignment::center);
        }

        void WindowNode::on_click(NodeInputContext& ctx)
        {
            if(!open) return;
            RectF close = close_rect(ctx.rect());
            Float2U pos = ctx.pointer_position();
            if(pos.x < close.offset_x || pos.x > close.offset_x + close.width ||
                pos.y < close.offset_y || pos.y > close.offset_y + close.height)
            {
                return;
            }
            *open = false;
            ctx.set_state(Name("gui.value_changed"), Any(true));
        }

        Guid PopupNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> PopupNode::clone() const
        {
            return new_object<PopupNode>(*this);
        }

        NodeLayerRole PopupNode::layer_role() const
        {
            return NodeLayerRole::popup;
        }

        bool PopupNode::default_interactive() const
        {
            return true;
        }

        bool PopupNode::uses_node_measure() const
        {
            return false;
        }

        void PopupNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.08f, 0.10f, 0.13f, 0.98f), 5.0f);
        }

        Guid TooltipNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> TooltipNode::clone() const
        {
            return new_object<TooltipNode>(*this);
        }

        NodeLayerRole TooltipNode::layer_role() const
        {
            return NodeLayerRole::tooltip;
        }

        bool TooltipNode::uses_node_measure() const
        {
            return false;
        }

        void TooltipNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.05f, 0.06f, 0.07f, 0.97f), 4.0f);
            Float4U border(0.28f, 0.33f, 0.40f, 1.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, rect.width, 1.0f), clip_rect, border, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + max(rect.height - 1.0f, 0.0f), rect.width, 1.0f), clip_rect, border, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y, 1.0f, rect.height), clip_rect, border, 0.0f);
            ctx.draw_rect(RectF(rect.offset_x + max(rect.width - 1.0f, 0.0f), rect.offset_y, 1.0f, rect.height), clip_rect, border, 0.0f);
        }

        MenuBarNode::MenuBarNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid MenuBarNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> MenuBarNode::clone() const
        {
            return new_object<MenuBarNode>(*this);
        }

        NodeLayoutFlow MenuBarNode::layout_flow() const
        {
            return NodeLayoutFlow::horizontal;
        }

        bool MenuBarNode::default_interactive() const
        {
            return true;
        }

        bool MenuBarNode::uses_node_measure() const
        {
            return false;
        }

        void MenuBarNode::apply_container_defaults(LayoutDesc& desc) const
        {
            desc.padding = EdgeInsets::xy(4.0f, 2.0f);
            desc.gap = 2.0f;
            desc.cross_axis_alignment = LayoutCrossAxisAlignment::center;
        }

        void MenuBarNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.08f, 0.10f, 0.13f, 0.92f), 0.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + max(rect.height - 1.0f, 0.0f), rect.width, 1.0f),
                clip_rect, Float4U(0.20f, 0.24f, 0.30f, 1.0f), 0.0f);
        }

        TableLayoutNode::TableLayoutNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid TableLayoutNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> TableLayoutNode::clone() const
        {
            return new_object<TableLayoutNode>(*this);
        }

        NodeLayoutBehavior TableLayoutNode::layout_behavior() const
        {
            return NodeLayoutBehavior::table;
        }

        bool TableLayoutNode::default_interactive() const
        {
            return true;
        }

        bool TableLayoutNode::uses_node_measure() const
        {
            return false;
        }

        bool TableLayoutNode::uses_context_render() const
        {
            return true;
        }

        GridLayoutNode::GridLayoutNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid GridLayoutNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> GridLayoutNode::clone() const
        {
            return new_object<GridLayoutNode>(*this);
        }

        NodeLayoutBehavior GridLayoutNode::layout_behavior() const
        {
            return NodeLayoutBehavior::grid;
        }

        bool GridLayoutNode::uses_node_measure() const
        {
            return false;
        }

        CanvasLayoutNode::CanvasLayoutNode()
        {
            layout_style = LayoutStyle::fill_width();
        }

        Guid CanvasLayoutNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> CanvasLayoutNode::clone() const
        {
            return new_object<CanvasLayoutNode>(*this);
        }

        NodeLayoutBehavior CanvasLayoutNode::layout_behavior() const
        {
            return NodeLayoutBehavior::canvas;
        }

        bool CanvasLayoutNode::uses_node_measure() const
        {
            return false;
        }

        void CanvasLayoutNode::apply_container_defaults(LayoutDesc& desc) const
        {
            desc.padding = EdgeInsets::all(0.0f);
            desc.gap = 0.0f;
        }

        DockSpaceNode::DockSpaceNode()
        {
            layout_style = LayoutStyle::fill();
        }

        Guid DockSpaceNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> DockSpaceNode::clone() const
        {
            return new_object<DockSpaceNode>(*this);
        }

        NodeLayoutBehavior DockSpaceNode::layout_behavior() const
        {
            return NodeLayoutBehavior::dock_space;
        }

        bool DockSpaceNode::uses_node_measure() const
        {
            return false;
        }

        void DockSpaceNode::apply_container_defaults(LayoutDesc& desc) const
        {
            desc.padding = EdgeInsets::all(0.0f);
            desc.gap = 0.0f;
        }

        void DockSpaceNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.07f, 0.08f, 0.10f, 1.0f), 0.0f);
        }

        TabBarNode::TabBarNode()
        {
            layout_style = LayoutStyle::fill();
        }

        Guid TabBarNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> TabBarNode::clone() const
        {
            return new_object<TabBarNode>(*this);
        }

        NodeLayoutBehavior TabBarNode::layout_behavior() const
        {
            return NodeLayoutBehavior::tab_bar;
        }

        bool TabBarNode::default_interactive() const
        {
            return true;
        }

        bool TabBarNode::uses_node_measure() const
        {
            return false;
        }

        void TabBarNode::apply_container_defaults(LayoutDesc& desc) const
        {
            desc.padding = EdgeInsets::all(0.0f);
            desc.gap = 0.0f;
        }

        void TabBarNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            ctx.draw_rect(rect, clip_rect, Float4U(0.08f, 0.10f, 0.13f, 0.70f), 4.0f);
            ctx.draw_rect(RectF(rect.offset_x, rect.offset_y + 31.0f, rect.width, 1.0f),
                clip_rect, Float4U(0.22f, 0.27f, 0.34f, 1.0f), 0.0f);
        }

        Guid TabItemNode::type_guid() const
        {
            return __guid;
        }

        Ref<Node> TabItemNode::clone() const
        {
            return new_object<TabItemNode>(*this);
        }

        NodeLayoutBehavior TabItemNode::layout_behavior() const
        {
            return NodeLayoutBehavior::tab_item;
        }

        bool TabItemNode::uses_node_measure() const
        {
            return false;
        }

        bool TabItemNode::uses_context_render() const
        {
            return true;
        }

    }
}
