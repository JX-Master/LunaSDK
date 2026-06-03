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
        struct ButtonGroupNode : Node
        {
            lustruct("GUI::ButtonGroupNode", "{66300361-8899-4285-977C-F39581E46C74}");

            i32* current_item = nullptr;
            bool* selected = nullptr;
            Vector<String> items;

            ButtonGroupNode();

            virtual Guid type_guid() const override;
            virtual Ref<Node> clone() const override;

            RectF item_rect(const RectF& rect, u32 index) const;

            i32 item_at(const RectF& rect, const Float2U& pos) const;

            virtual LayoutMetrics measure() const override;

            virtual void on_click(NodeInputContext& ctx) override;
        };
    }
}
