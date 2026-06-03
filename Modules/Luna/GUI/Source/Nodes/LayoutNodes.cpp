/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "LayoutNodes.hpp"
#include "../RenderProxies/LayoutRenderProxies.hpp"
#include "../RenderProxies/TableRenderProxies.hpp"

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

        ScrollViewNode::ScrollViewNode()
        {
            render_proxy = default_scroll_view_render_proxy();
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

        WindowNode::WindowNode()
        {
            render_proxy = default_window_render_proxy();
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

        PopupNode::PopupNode()
        {
            render_proxy = default_popup_render_proxy();
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

        TooltipNode::TooltipNode()
        {
            render_proxy = default_tooltip_render_proxy();
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

        MenuBarNode::MenuBarNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_menu_bar_render_proxy();
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

        TableLayoutNode::TableLayoutNode()
        {
            layout_style = LayoutStyle::fill_width();
            render_proxy = default_table_layout_render_proxy();
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
            render_proxy = default_dock_space_render_proxy();
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

        TabBarNode::TabBarNode()
        {
            layout_style = LayoutStyle::fill();
            render_proxy = default_tab_bar_render_proxy();
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

        TabItemNode::TabItemNode()
        {
            render_proxy = default_tab_item_render_proxy();
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

    }
}
