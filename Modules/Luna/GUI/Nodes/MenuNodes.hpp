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
        struct MenuSeparatorNode : Node
        {
            lustruct("GUI::MenuSeparatorNode", "{B96A93F9-26E9-49B4-951B-99419867EB39}");

            MenuSeparatorNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            virtual LayoutMetrics measure() const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
        };
        struct MenuItemNode : Node
        {
            lustruct("GUI::MenuItemNode", "{4EF08EDA-74E4-4E56-A871-F373D6424F37}");

            String shortcut;
            bool selected = false;
            bool* selected_value = nullptr;
            bool enabled = true;
            bool top_level_menu = false;
            id_t popup_id = 0;

            MenuItemNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;
            virtual bool enabled_state() const override;
            virtual id_t menu_popup() const override;
            virtual void set_menu_popup(id_t value) override;

            bool checked() const;

            virtual LayoutMetrics measure() const override;
            virtual LayoutMetrics measure(NodeMeasureContext& ctx) const override;

            virtual void render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const override;
            virtual void update_state(NodeInputContext& ctx) const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
    }
}
