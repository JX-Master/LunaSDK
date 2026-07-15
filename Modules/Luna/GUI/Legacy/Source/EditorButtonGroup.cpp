/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorButtonGroup.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "EditorInternal.hpp"
#include <Luna/GUI/Legacy/EditorState.hpp>
#include <Luna/GUI/Legacy/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry, const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void set_basic_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element,
            bool enabled = true)
        {
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            set_flags(interactable.flags, GUICore::InteractableFlag::disabled, !enabled);
            context->set_interactable(element, interactable);
        }

        static void draw_relative_rect(GUICore::IContext* context, GUICore::DrawCommandType type, const RectF& rect,
            const Float4U& color, f32 radius = 0.0f, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.radius = radius;
            context->draw(command);
        }

        static void draw_relative_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
            const Float4U& color, f32 width, const Float4U& scale = Float4U(0.0f))
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::line;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(begin.x, begin.y, 0.0f, 0.0f);
            command.point1 = end;
            command.rect_layout_scale = scale;
            command.color = color;
            command.line_width = width;
            context->draw(command);
        }

        static void draw_scaled_text(GUICore::IContext* context, const RectF& rect, const Float4U& scale,
            const c8* text, const Float4U& color, f32 font_size, VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = horizontal_alignment;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static Float4U smooth_color(const Float4U& from, const Float4U& to, f32 t)
        {
            t = clamp(t, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t);
            return from + (to - from) * t;
        }

        static i32 button_group_item_at(const RectF& rect, const Float2U& pos, u32 count)
        {
            if(!count || rect.width <= 0.0f || rect.height <= 0.0f ||
                pos.x < 0.0f || pos.x > rect.width || pos.y < 0.0f || pos.y > rect.height)
            {
                return -1;
            }
            f32 item_width = max(rect.width / (f32)count, 1.0f);
            i32 item = (i32)(pos.x / item_width);
            return clamp(item, 0, (i32)count - 1);
        }

        static void button_group_apply_click(GUICore::IContext* context, GUICore::id_t id, i32* current_item,
            Span<bool> selected, u32 item_count, bool enabled)
        {
            if(!enabled)
            {
                return;
            }
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            if(!interaction.clicked)
            {
                return;
            }
            i32 item = button_group_item_at(interaction.clicked_element_rect, interaction.clicked_element_position, item_count);
            if(item < 0)
            {
                return;
            }
            if(current_item)
            {
                *current_item = item;
            }
            else if((u32)item < selected.size())
            {
                selected[item] = !selected[item];
            }
        }

        static GUICore::ElementHandle button_group_impl(GUICore::IContext* context, GUICore::id_t id, i32* current_item,
            Span<bool> selected, Span<const c8*> items, const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            u32 count = (u32)items.size();
            button_group_apply_click(context, id, current_item, selected, count, enabled);

            GUICore::ElementHandle element = Internal::begin_element(context, id, "button_group");
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element, enabled);
            if(!count)
            {
                context->end_element();
                return element;
            }

            GUICore::InteractionState interaction = context->get_interaction_state(id);
            i32 hover_item = interaction.hovered ?
                button_group_item_at(interaction.pointer_element_rect, interaction.pointer_element_position, count) : -1;
            i32 active_item = interaction.active ?
                button_group_item_at(interaction.pointer_element_rect, interaction.pointer_element_position, count) : -1;

            f32 radius = style_value(context, Name("gui.button_group.radius"), GUICore::style_f32(5.0f)).number.x;
            Float4U border_color = style_value(context, Name("gui.button_group.border"),
                GUICore::style_f32x4(Float4U(0.25f, 0.29f, 0.35f, 1.0f))).number;
            Float4U bg_color = style_value(context, Name("gui.button_group.background"),
                GUICore::style_f32x4(Float4U(0.07f, 0.08f, 0.10f, 1.0f))).number;
            Float4U selected_color = style_value(context, Name("gui.button_group.selected"),
                GUICore::style_f32x4(Float4U(0.16f, 0.24f, 0.38f, 1.0f))).number;
            Float4U selected_hot_color = style_value(context, Name("gui.button_group.selected_hot"),
                GUICore::style_f32x4(Float4U(0.20f, 0.33f, 0.54f, 1.0f))).number;
            Float4U hover_color = style_value(context, Name("gui.button_group.hover"),
                GUICore::style_f32x4(Float4U(0.14f, 0.17f, 0.22f, 1.0f))).number;
            Float4U separator_color = style_value(context, Name("gui.button_group.separator"),
                GUICore::style_f32x4(Float4U(0.20f, 0.23f, 0.28f, 0.90f))).number;
            Float4U selected_text_color = style_value(context, Name("gui.button_group.text_selected"),
                GUICore::style_f32x4(Float4U(1.0f))).number;
            Float4U text_color = style_value(context, Name("gui.button_group.text"),
                GUICore::style_f32x4(Float4U(0.58f, 0.63f, 0.70f, 1.0f))).number;
            if(!enabled)
            {
                selected_color = style_value(context, Name("gui.button_group.selected_disabled"),
                    GUICore::style_f32x4(Float4U(0.16f, 0.18f, 0.22f, 1.0f))).number;
                selected_hot_color = selected_color;
                hover_color = bg_color;
                selected_text_color = style_value(context, Name("gui.button_group.text_disabled"),
                    GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
                text_color = selected_text_color;
            }
            f32 font_size = style_value(context, Name("gui.button_group.font_size"), GUICore::style_f32(15.0f)).number.x;
            f32 animation_speed = style_value(context, Name("gui.button_group.animation_speed"), GUICore::style_f32(14.0f)).number.x;
            f32 blend = clamp(context->get_frame_desc().delta_time * animation_speed, 0.0f, 1.0f);

            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(0.0f, 0.0f, 0.0f, 0.0f), border_color, radius);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(1.0f, 1.0f, -2.0f, -2.0f), bg_color, max(radius - 1.0f, 0.0f));

            id_t state_id = GUICore::make_state_id<ButtonGroupAnimationState>(id);
            ButtonGroupAnimationState* previous_state = nullptr;
            if(object_t state_obj = context->get_state(state_id))
            {
                previous_state = cast_object<ButtonGroupAnimationState>(state_obj);
            }
            Ref<ButtonGroupAnimationState> next_state = new_object<ButtonGroupAnimationState>();
            if(current_item)
            {
                f32 target = (f32)clamp(*current_item, 0, (i32)count - 1);
                f32 animation = target;
                if(previous_state && previous_state->selection_animation_initialized)
                {
                    animation = previous_state->selection_animation;
                }
                animation += (target - animation) * blend;
                next_state->selection_animation = animation;
                next_state->selection_animation_initialized = true;
                f32 scale_x = animation / (f32)count;
                f32 scale_w = 1.0f / (f32)count;
                GUICore::DrawCommand selection;
                selection.type = GUICore::DrawCommandType::rounded_rect;
                selection.rect_reference = GUICore::DrawCommandRectReference::element;
                selection.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                selection.rect_layout_scale = Float4U(scale_x, 0.0f, scale_w, 1.0f);
                selection.color = active_item == (i32)target ? selected_hot_color : selected_color;
                selection.radius = max(radius - 1.0f, 0.0f);
                context->draw(selection);
            }
            else
            {
                if(previous_state)
                {
                    next_state->item_animations = previous_state->item_animations;
                }
                if(next_state->item_animations.size() != count)
                {
                    next_state->item_animations.assign(count, 0.0f);
                    for(u32 i = 0; i < count; ++i)
                    {
                        next_state->item_animations[i] = (i < selected.size() && selected[i]) ? 1.0f : 0.0f;
                    }
                }
                for(u32 i = 0; i < count; ++i)
                {
                    f32 target = (i < selected.size() && selected[i]) ? 1.0f : 0.0f;
                    f32& animation = next_state->item_animations[i];
                    animation += (target - animation) * blend;
                    f32 t = clamp(animation, 0.0f, 1.0f);
                    if(t > 0.001f || hover_item == (i32)i || active_item == (i32)i)
                    {
                        f32 scale_x = (f32)i / (f32)count;
                        f32 scale_w = 1.0f / (f32)count;
                        Float4U base_color = (hover_item == (i32)i || active_item == (i32)i) ? hover_color : bg_color;
                        GUICore::DrawCommand fill;
                        fill.type = GUICore::DrawCommandType::rounded_rect;
                        fill.rect_reference = GUICore::DrawCommandRectReference::element;
                        fill.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                        fill.rect_layout_scale = Float4U(scale_x, 0.0f, scale_w, 1.0f);
                        fill.color = smooth_color(base_color, selected_color, t);
                        fill.radius = max(radius - 1.0f, 0.0f);
                        context->draw(fill);
                    }
                }
            }
            lupanic_if_failed(context->set_state(state_id, next_state.object(), GUICore::StateLifetime::next_frame));

            if(current_item && hover_item >= 0 && hover_item != *current_item)
            {
                f32 scale_x = (f32)hover_item / (f32)count;
                f32 scale_w = 1.0f / (f32)count;
                GUICore::DrawCommand hover;
                hover.type = GUICore::DrawCommandType::rounded_rect;
                hover.rect_reference = GUICore::DrawCommandRectReference::element;
                hover.rect = RectF(1.0f, 1.0f, -2.0f, -2.0f);
                hover.rect_layout_scale = Float4U(scale_x, 0.0f, scale_w, 1.0f);
                hover.color = hover_color;
                hover.radius = max(radius - 1.0f, 0.0f);
                context->draw(hover);
            }
            for(u32 i = 1; i < count; ++i)
            {
                f32 scale_x = (f32)i / (f32)count;
                draw_relative_line(context, Float2U(0.0f, 2.0f), Float2U(0.0f, -2.0f), separator_color, 1.0f,
                    Float4U(scale_x, 0.0f, scale_x, 1.0f));
            }
            for(u32 i = 0; i < count; ++i)
            {
                bool item_selected = current_item ? *current_item == (i32)i : (i < selected.size() && selected[i]);
                f32 scale_x = (f32)i / (f32)count;
                f32 scale_w = 1.0f / (f32)count;
                draw_scaled_text(context, RectF(8.0f, 0.0f, -16.0f, 0.0f),
                    Float4U(scale_x, 0.0f, scale_w, 1.0f), items[i] ? items[i] : "",
                    item_selected ? selected_text_color : text_color, font_size, VG::TextAlignment::center);
            }
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, GUICore::id_t id,
            i32* current_item, Span<const c8*> items, const GUICore::LayoutConfig& layout, bool enabled)
        {
            return button_group_impl(context, id, current_item, Span<bool>(), items, layout, enabled);
        }

        LUNA_GUI_API GUICore::ElementHandle button_group(GUICore::IContext* context, GUICore::id_t id,
            Span<bool> selected, Span<const c8*> items, const GUICore::LayoutConfig& layout, bool enabled)
        {
            return button_group_impl(context, id, nullptr, selected, items, layout, enabled);
        }
    }
}
