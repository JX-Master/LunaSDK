/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"

namespace Luna
{
    namespace GUI
    {
        struct RootNode : Node
        {
            lustruct("GUI::RootNode", "{24EF0005-B823-4C28-929C-987F18238ABF}");

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayerRole layer_role() const override;
            virtual bool uses_node_measure() const override;
        };
        struct HLayoutNode : Node
        {
            lustruct("GUI::HLayoutNode", "{34171215-492A-4321-BDEA-371F6BC90996}");

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutFlow layout_flow() const override;
            virtual bool uses_node_measure() const override;
        };
        struct VLayoutNode : Node
        {
            lustruct("GUI::VLayoutNode", "{629AF481-BA18-4CB8-93ED-9D1EE652D21C}");

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool uses_node_measure() const override;
        };
        struct ScrollViewNode : Node
        {
            lustruct("GUI::ScrollViewNode", "{99956C7E-0997-4E23-8A95-373C5196A7F7}");

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool default_interactive() const override;
            virtual bool uses_node_measure() const override;
            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct WindowNode : Node
        {
            lustruct("GUI::WindowNode", "{5EB73D33-D324-43E6-AB3F-502BBCD40CDE}");

            bool* open = nullptr;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool uses_node_measure() const override;
            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            static f32 title_bar_height();

            static RectF close_rect(const RectF& rect);

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct PopupNode : Node
        {
            lustruct("GUI::PopupNode", "{4F3DD901-DAA3-40DB-84C1-5115157C52D8}");

            PopupFlag flags = PopupFlag::none;
            id_t parent_popup = 0;
            id_t owner = 0;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayerRole layer_role() const override;
            virtual bool default_interactive() const override;
            virtual bool uses_node_measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct TooltipNode : Node
        {
            lustruct("GUI::TooltipNode", "{2ADD9350-26DF-4D76-8A54-940C213F8730}");

            TooltipDesc desc;
            id_t owner = 0;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayerRole layer_role() const override;
            virtual bool uses_node_measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct MenuBarNode : Node
        {
            lustruct("GUI::MenuBarNode", "{6ED53028-CB49-4A5F-857D-B9C63B4C133C}");

            MenuBarNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutFlow layout_flow() const override;
            virtual bool default_interactive() const override;
            virtual bool uses_node_measure() const override;
            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct TableLayoutNode : Node
        {
            lustruct("GUI::TableLayoutNode", "{173F13E1-D89C-4298-8EA4-74400F85C11F}");

            TableDesc desc;

            TableLayoutNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool default_interactive() const override;
            virtual bool uses_node_measure() const override;
            virtual bool uses_context_render() const override;
        };
        struct GridLayoutNode : Node
        {
            lustruct("GUI::GridLayoutNode", "{7255CE2D-5213-43B4-980B-3DB1FBF0A727}");

            GridLayoutDesc desc;

            GridLayoutNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool uses_node_measure() const override;
        };
        struct CanvasLayoutNode : Node
        {
            lustruct("GUI::CanvasLayoutNode", "{3C74E39A-09C3-40C6-A612-DD27B0E4BB9A}");

            CanvasLayoutDesc desc;

            CanvasLayoutNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool uses_node_measure() const override;
            virtual void apply_container_defaults(LayoutDesc& desc) const override;
        };
        struct DockSpaceNode : Node
        {
            lustruct("GUI::DockSpaceNode", "{1FEE4004-7AE1-4B09-A76B-5853159CB940}");

            DockSpaceNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool uses_node_measure() const override;
            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct TabBarNode : Node
        {
            lustruct("GUI::TabBarNode", "{20647252-F597-4C02-A465-5E2B274F1587}");

            TabBarFlag flags = TabBarFlag::none;

            TabBarNode();
            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool default_interactive() const override;
            virtual bool uses_node_measure() const override;
            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct TabItemNode : Node
        {
            lustruct("GUI::TabItemNode", "{3F9D09AA-938D-4991-9E09-0EC593173DB1}");

            bool* open = nullptr;
            TabItemFlag flags = TabItemFlag::none;
            bool content_visible = false;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual NodeLayoutBehavior layout_behavior() const override;
            virtual bool uses_node_measure() const override;
            virtual bool uses_context_render() const override;
        };
    }
}
