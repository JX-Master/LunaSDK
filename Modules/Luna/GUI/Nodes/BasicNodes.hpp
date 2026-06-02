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
        struct ButtonNode : Node
        {
            lustruct("GUI::ButtonNode", "{1AA61495-B39A-4109-971F-4ABDB810F4EE}");

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct TextNode : Node
        {
            lustruct("GUI::TextNode", "{686DF821-591B-41C4-BC10-9420C24FE28D}");

            Float4U color = Float4U(1.0f);
            f32 font_size = 16.0f;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual LayoutMetrics measure(NodeMeasureContext& ctx) const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct SelectableNode : Node
        {
            lustruct("GUI::SelectableNode", "{F021AFD7-2AC6-4CCC-B6B5-D4BE6FDDF0E4}");

            bool selected = false;

            SelectableNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct CheckboxNode : Node
        {
            lustruct("GUI::CheckboxNode", "{967B664F-1718-4F2A-8EA0-82879DA3579B}");

            bool* value = nullptr;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct ToggleSwitchNode : Node
        {
            lustruct("GUI::ToggleSwitchNode", "{E7794BFA-1A37-4890-BDFD-0C15F5CC8942}");

            bool* value = nullptr;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct CollapsingHeaderNode : Node
        {
            lustruct("GUI::CollapsingHeaderNode", "{3F6E2788-AE42-4B9C-8309-742C8ACC5933}");

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            bool open(NodeInputContext& ctx) const;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void update_state(NodeInputContext& ctx) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct TreeNodeNode : Node
        {
            lustruct("GUI::TreeNodeNode", "{55ACA6E4-97CA-4D42-BF56-EABE7F02DC48}");

            TreeNodeFlag flags = TreeNodeFlag::none;
            u32 indent_depth = 0;
            bool selected = false;

            TreeNodeNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            bool leaf() const;

            bool open(NodeInputContext& ctx) const;

            RectF arrow_rect(const RectF& rect) const;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void update_state(NodeInputContext& ctx) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct RadioButtonNode : Node
        {
            lustruct("GUI::RadioButtonNode", "{EB54B8B9-8C68-4072-B632-750291A8569A}");

            bool selected = false;
            bool* value = nullptr;
            i32* i32_value = nullptr;
            i32 item_value = 0;

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            bool selected_state() const;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
    }
}
