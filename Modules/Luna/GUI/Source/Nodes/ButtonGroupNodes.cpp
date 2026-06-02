/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/ButtonGroupNodes.hpp"
#include "../../State.hpp"

namespace Luna
{
    namespace GUI
    {
        Guid ButtonGroupNode::type_guid() const
        {
            return __guid;
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

        void ButtonGroupNode::render(NodeRenderContext& ctx, const RectF& rect, const RectF& clip_rect, const NodeRenderState& state) const
        {
            u32 count = (u32)items.size();
            if(!count) return;
            i32 hover_item = state.hovered ? item_at(rect, state.pointer_position) : -1;
            i32 active_item = state.active ? item_at(rect, state.pointer_position) : -1;
            f32 radius = min(5.0f, min(rect.width, rect.height) * 0.5f);
            RectF inner(rect.offset_x + 1.0f, rect.offset_y + 1.0f, max(rect.width - 2.0f, 1.0f), max(rect.height - 2.0f, 1.0f));
            f32 inner_radius = max(radius - 1.0f, 0.0f);
            Float4U border_color = Float4U(0.25f, 0.29f, 0.35f, 1.0f);
            Float4U bg_color = Float4U(0.07f, 0.08f, 0.10f, 1.0f);
            Float4U selected_color = Float4U(0.16f, 0.24f, 0.38f, 1.0f);
            Float4U selected_hot_color = Float4U(0.20f, 0.33f, 0.54f, 1.0f);
            Float4U hover_color = Float4U(0.14f, 0.17f, 0.22f, 1.0f);

            ctx.draw_rect(rect, clip_rect, border_color, radius);
            ctx.draw_rect(inner, clip_rect, bg_color, inner_radius);

            f32 blend = clamp(state.delta_time * 14.0f, 0.0f, 1.0f);
            if(current_item)
            {
                f32 target = (f32)clamp(*current_item, 0, (i32)count - 1);
                f32 selection_animation = target;
                ButtonGroupAnimationState* animation_state = ctx.get_widget_state<ButtonGroupAnimationState>(id);
                if(animation_state && animation_state->selection_animation_initialized)
                {
                    selection_animation = animation_state->selection_animation;
                }
                selection_animation += (target - selection_animation) * blend;
                Ref<ButtonGroupAnimationState> next_animation_state = ctx.get_or_create_widget_state<ButtonGroupAnimationState>(id);
                next_animation_state->selection_animation = selection_animation;
                next_animation_state->selection_animation_initialized = true;
                f32 item_width = inner.width / (f32)count;
                RectF selection_rect(inner.offset_x + item_width * selection_animation, inner.offset_y, item_width, inner.height);
                f32 max_x = inner.offset_x + inner.width;
                if(selection_rect.offset_x + selection_rect.width > max_x)
                {
                    selection_rect.width = max(max_x - selection_rect.offset_x, 1.0f);
                }
                ctx.draw_rect(selection_rect, clip_rect, active_item == (i32)target ? selected_hot_color : selected_color, inner_radius);
            }
            else if(selected)
            {
                Vector<f32> animations;
                ButtonGroupAnimationState* animation_state = ctx.get_widget_state<ButtonGroupAnimationState>(id);
                if(animation_state)
                {
                    animations = animation_state->item_animations;
                }
                if(animations.size() != count)
                {
                    animations.assign(count, 0.0f);
                    for(u32 i = 0; i < count; ++i)
                    {
                        animations[i] = selected[i] ? 1.0f : 0.0f;
                    }
                }
                for(u32 i = 0; i < count; ++i)
                {
                    f32 target = selected[i] ? 1.0f : 0.0f;
                    animations[i] += (target - animations[i]) * blend;
                    f32 t = clamp(animations[i], 0.0f, 1.0f);
                    RectF button_rect = item_rect(inner, i);
                    Float4U base_color = (active_item == (i32)i || hover_item == (i32)i) ? hover_color : bg_color;
                    if(t > 0.001f || hover_item == (i32)i || active_item == (i32)i)
                    {
                        ctx.draw_rect_corners(button_rect, clip_rect, smooth_color(base_color, selected_color, t), inner_radius,
                            i == 0, i + 1 == count, i + 1 == count, i == 0);
                    }
                }
                Ref<ButtonGroupAnimationState> next_animation_state = ctx.get_or_create_widget_state<ButtonGroupAnimationState>(id);
                next_animation_state->item_animations = move(animations);
            }
            if(current_item && hover_item >= 0 && hover_item != *current_item)
            {
                RectF button_rect = item_rect(inner, (u32)hover_item);
                ctx.draw_rect_corners(button_rect, clip_rect, hover_color, inner_radius,
                    hover_item == 0, (u32)hover_item + 1 == count, (u32)hover_item + 1 == count, hover_item == 0);
            }
            for(u32 i = 1; i < count; ++i)
            {
                f32 x = rect.offset_x + rect.width * ((f32)i / (f32)count);
                ctx.draw_line(Float2U(x, rect.offset_y + 2.0f), Float2U(x, rect.offset_y + max(rect.height - 2.0f, 2.0f)),
                    clip_rect, Float4U(0.20f, 0.23f, 0.28f, 0.90f), 1.0f);
            }
            for(u32 i = 0; i < count; ++i)
            {
                RectF button_rect = item_rect(inner, i);
                bool item_selected = current_item ? *current_item == (i32)i : (selected && selected[i]);
                Float4U text_color = item_selected ? Float4U(1.0f) : Float4U(0.58f, 0.63f, 0.70f, 1.0f);
                ctx.draw_text(RectF(button_rect.offset_x + 8.0f, button_rect.offset_y, max(button_rect.width - 16.0f, 1.0f), button_rect.height),
                    clip_rect, items[i].c_str(), 15.0f, text_color, TextAlignment::center);
            }
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
