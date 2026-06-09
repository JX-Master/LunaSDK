/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Common.hpp"
#include "BasicNodes.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        struct [[Luna::struct("{1AA61495-B39A-4109-971F-4ABDB810F4EE}")]] ButtonNode : Node
        {
            ButtonNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

        };
        struct [[Luna::struct("{FD7AF111-3102-42FD-95B7-38E82E461D50}")]] ProgressBarNode : Node
        {
            f32 fraction = 0.0f;
            String overlay;
            bool has_overlay = false;

            ProgressBarNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

        };
        struct [[Luna::struct("{686DF821-591B-41C4-BC10-9420C24FE28D}")]] TextNode : Node
        {
            Float4U color = Float4U(1.0f);
            f32 font_size = 16.0f;

            TextNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual LayoutMetrics measure(NodeMeasureContext& ctx) const override;

        };
        struct [[Luna::struct("{F021AFD7-2AC6-4CCC-B6B5-D4BE6FDDF0E4}")]] SelectableNode : Node
        {
            bool selected = false;
            bool label_layout = false;

            SelectableNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual NodeLayoutFlow layout_flow() const override;

            virtual bool default_interactive() const override;

            virtual bool uses_node_measure() const override;

            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual LayoutMetrics measure() const override;

        };
        struct [[Luna::struct("{967B664F-1718-4F2A-8EA0-82879DA3579B}")]] CheckboxNode : Node
        {
            bool* value = nullptr;
            bool label_layout = false;

            CheckboxNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual NodeLayoutFlow layout_flow() const override;

            virtual bool default_interactive() const override;

            virtual bool uses_node_measure() const override;

            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual LayoutMetrics measure() const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct [[Luna::struct("{E7794BFA-1A37-4890-BDFD-0C15F5CC8942}")]] ToggleSwitchNode : Node
        {
            bool* value = nullptr;
            bool label_layout = false;

            ToggleSwitchNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual NodeLayoutFlow layout_flow() const override;

            virtual bool default_interactive() const override;

            virtual bool uses_node_measure() const override;

            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual LayoutMetrics measure() const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct [[Luna::struct("{3F6E2788-AE42-4B9C-8309-742C8ACC5933}")]] CollapsingHeaderNode : Node
        {
            CollapsingHeaderNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            bool open(NodeInputContext& ctx) const;

            virtual LayoutMetrics measure() const override;

            virtual void update_state(NodeInputContext& ctx) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct [[Luna::struct("{55ACA6E4-97CA-4D42-BF56-EABE7F02DC48}")]] TreeNodeNode : Node
        {
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

            virtual void update_state(NodeInputContext& ctx) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
        struct [[Luna::struct("{EB54B8B9-8C68-4072-B632-750291A8569A}")]] RadioButtonNode : Node
        {
            bool selected = false;
            bool* value = nullptr;
            i32* i32_value = nullptr;
            i32 item_value = 0;
            bool label_layout = false;

            RadioButtonNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            bool selected_state() const;

            virtual NodeLayoutFlow layout_flow() const override;

            virtual bool default_interactive() const override;

            virtual bool uses_node_measure() const override;

            virtual void apply_container_defaults(LayoutDesc& desc) const override;

            virtual LayoutMetrics measure() const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
    }
}
