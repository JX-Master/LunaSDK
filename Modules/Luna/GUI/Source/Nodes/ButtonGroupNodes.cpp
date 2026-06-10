/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "ButtonGroupNodes.hpp"
#include "../../State.hpp"
#include "../RenderProxies/BasicRenderProxies.hpp"

namespace Luna
{
    namespace GUI
    {
        ButtonGroupNode::ButtonGroupNode()
        {
            render_proxy = default_button_group_render_proxy();
        }

        Guid ButtonGroupNode::type_guid() const
        {
            return Meta::StructMetaData<ButtonGroupNode>::__guid;
        }

        Ref<Node> ButtonGroupNode::clone() const
        {
            return new_object<ButtonGroupNode>(*this);
        }

        RectF ButtonGroupNode::item_rect(const RectF& rect, u32 index) const
        {
            u32 count = max((u32)items.size(), 1u);
            f32 item_width = rect.width / (f32)count;
            f32 x = rect.offset_x + item_width * (f32)index;
            f32 w = index + 1 == count ? max(rect.offset_x + rect.width - x, 1.0f) : max(item_width, 1.0f);
            return RectF(x, rect.offset_y, w, rect.height);
        }

        i32 ButtonGroupNode::item_at(const RectF& rect, const Float2U& pos) const
        {
            u32 count = (u32)items.size();
            if(!count ||
                pos.x < rect.offset_x || pos.x > rect.offset_x + rect.width ||
                pos.y < rect.offset_y || pos.y > rect.offset_y + rect.height)
            {
                return -1;
            }
            f32 item_width = max(rect.width / (f32)count, 1.0f);
            i32 index = (i32)((pos.x - rect.offset_x) / item_width);
            return index >= 0 && (u32)index < count ? index : (i32)count - 1;
        }

        LayoutMetrics ButtonGroupNode::measure() const
        {
            f32 width = 0.0f;
            for(const String& item : items)
            {
                width += max((f32)item.size() * 16.0f * 0.52f + 32.0f, 76.0f);
            }
            if(items.empty()) width = 76.0f;
            LayoutMetrics metrics;
            metrics.min_size = Float2U(max((f32)items.size() * 44.0f, 44.0f), 28.0f);
            metrics.preferred_size = Float2U(max(width, metrics.min_size.x), 28.0f);
            metrics.max_size = Float2U(F32_MAX, 28.0f);
            return metrics;
        }

        void ButtonGroupNode::on_click(NodeInputContext& ctx)
        {
            i32 item = item_at(ctx.rect(), ctx.pointer_position());
            if(item < 0) return;
            if(current_item)
            {
                if(*current_item != item)
                {
                    *current_item = item;
                    ctx.set_state(Name("gui.value_changed"), Any(true));
                }
            }
            else if(selected)
            {
                selected[item] = !selected[item];
                ctx.set_state(Name("gui.value_changed"), Any(true));
            }
        }

    }
}
