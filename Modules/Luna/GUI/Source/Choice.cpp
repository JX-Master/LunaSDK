/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Choice.cpp
* @author JXMaster
* @date 2026/7/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            enum class ChoiceKind : u8
            {
                selectable,
                checkbox,
                radio,
                toggle
            };

            struct ChoiceData
            {
                c8* label = nullptr;
                ChoiceKind kind = ChoiceKind::selectable;
                bool enabled = true;
                ChoiceVisualState* state = nullptr;
            };

            static Float4U mix_color(const Float4U& from, const Float4U& to, f32 amount)
            {
                amount = clamp(amount, 0.0f, 1.0f);
                amount = amount * amount * (3.0f - 2.0f * amount);
                return from + (to - from) * amount;
            }

            static GUICore::MeasureResult measure_choice(GUICore::IContext* context,
                const GUICore::ElementHandle& element,
                const Float2U&, void*)
            {
                f32 height = style_scalar(context, element, "gui.control.height", 28.0f);
                GUICore::MeasureResult result;
                result.minimum = Float2U(28.0f, height);
                result.desired = Float2U(140.0f, height);
                return result;
            }

            static void draw_rect(GUICore::IContext* context, const RectF& rect, const Float4U& scale,
                const Float4U& color, f32 radius)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = rect;
                command.rect_layout_scale = scale;
                command.color = color;
                command.radius = radius;
                context->draw(command);
            }

            static void draw_line(GUICore::IContext* context, const Float2U& begin, const Float2U& end,
                const Float4U& scale, const Float4U& color, f32 width)
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

            static RV draw_choice(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ChoiceData* data = (ChoiceData*)userdata;
                if(!data || !data->state) return ok;
                f32 selected = data->state->selected;
                Float4U text_color = data->enabled ? style_color(context, element, "gui.text.color",
                    Float4U(0.86f, 0.88f, 0.92f, 1.0f)) : style_color(context, element, "gui.text.disabled",
                    Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                const f32 indicator_size = style_scalar(context, element, "gui.choice.indicator_size", 18.0f);
                f32 label_offset = indicator_size + 8.0f;
                if(data->kind == ChoiceKind::selectable)
                {
                    Float4U background = style_color(context, element, "gui.choice.selected",
                        Float4U(0.16f, 0.30f, 0.48f, 1.0f));
                    Float4U hovered = style_color(context, element, "gui.choice.hovered",
                        Float4U(0.13f, 0.19f, 0.27f, 1.0f));
                    Float4U color = mix_color(Float4U(0.0f), hovered, data->state->hovered);
                    color = mix_color(color, background, selected);
                    if(color.w > 0.001f) draw_rect(context, RectF(), Float4U(), color, 4.0f);
                    label_offset = 8.0f;
                }
                else if(data->kind == ChoiceKind::checkbox)
                {
                    Float4U border = style_color(context, element, "gui.choice.border",
                        Float4U(0.55f, 0.64f, 0.76f, 1.0f));
                    Float4U fill = style_color(context, element, "gui.choice.accent",
                        Float4U(0.18f, 0.42f, 0.72f, 1.0f));
                    if(!data->enabled) border = fill = style_color(context, element, "gui.choice.disabled",
                        Float4U(0.30f, 0.34f, 0.40f, 1.0f));
                    const f32 half = indicator_size * 0.5f;
                    const f32 inset = indicator_size >= 20.0f ? 3.0f : 2.0f;
                    draw_rect(context, RectF(0.0f, -half, indicator_size, indicator_size),
                        Float4U(0.0f, 0.5f, 0.0f, 0.0f), border, 3.0f);
                    draw_rect(context, RectF(inset, -half + inset, indicator_size - inset * 2.0f,
                        indicator_size - inset * 2.0f),
                        Float4U(0.0f, 0.5f, 0.0f, 0.0f),
                        mix_color(style_color(context, element, "gui.choice.background",
                            Float4U(0.08f, 0.10f, 0.13f, 1.0f)), fill, selected), 2.0f);
                    if(selected > 0.02f)
                    {
                        Float4U mark = style_color(context, element, "gui.choice.mark", Float4U(1.0f));
                        mark.w *= selected;
                        const f32 scale = indicator_size / 16.0f;
                        draw_line(context, Float2U(4.0f * scale, -1.0f * scale),
                            Float2U(7.0f * scale, 3.0f * scale),
                            Float4U(0.0f, 0.5f, 0.0f, 0.5f), mark, 2.0f);
                        draw_line(context, Float2U(7.0f * scale, 3.0f * scale),
                            Float2U(13.0f * scale, -4.0f * scale),
                            Float4U(0.0f, 0.5f, 0.0f, 0.5f), mark, 2.0f);
                    }
                }
                else if(data->kind == ChoiceKind::radio)
                {
                    Float4U border = data->enabled ? style_color(context, element, "gui.choice.border",
                        Float4U(0.55f, 0.64f, 0.76f, 1.0f)) : style_color(context, element,
                        "gui.choice.disabled", Float4U(0.30f, 0.34f, 0.40f, 1.0f));
                    const f32 half = indicator_size * 0.5f;
                    const f32 inset = indicator_size >= 20.0f ? 3.0f : 2.0f;
                    draw_rect(context, RectF(0.0f, -half, indicator_size, indicator_size),
                        Float4U(0.0f, 0.5f, 0.0f, 0.0f), border, half);
                    draw_rect(context, RectF(inset, -half + inset, indicator_size - inset * 2.0f,
                        indicator_size - inset * 2.0f),
                        Float4U(0.0f, 0.5f, 0.0f, 0.0f), style_color(context, element,
                        "gui.choice.background", Float4U(0.08f, 0.10f, 0.13f, 1.0f)), half - inset);
                    if(selected > 0.02f)
                    {
                        Float4U fill = style_color(context, element, "gui.choice.accent",
                            Float4U(0.18f, 0.42f, 0.72f, 1.0f));
                        fill.w *= selected;
                        const f32 dot = indicator_size * 0.38f;
                        draw_rect(context, RectF((indicator_size - dot) * 0.5f, -dot * 0.5f, dot, dot),
                            Float4U(0.0f, 0.5f, 0.0f, 0.0f), fill, dot * 0.5f);
                    }
                }
                else
                {
                    const Float2U switch_size = style_vector2(context, element, "gui.switch.size",
                        Float2U(46.0f, 24.0f));
                    label_offset = switch_size.x + 10.0f;
                    Float4U off = style_color(context, element, "gui.switch.off",
                        Float4U(0.14f, 0.16f, 0.19f, 1.0f));
                    Float4U on = style_color(context, element, "gui.switch.on",
                        Float4U(0.18f, 0.52f, 0.34f, 1.0f));
                    Float4U knob = data->enabled ? Float4U(0.96f, 0.97f, 0.99f, 1.0f) :
                        style_color(context, element, "gui.choice.disabled", Float4U(0.4f, 0.44f, 0.5f, 1.0f));
                    if(!data->enabled) off = on = style_color(context, element, "gui.choice.background",
                        Float4U(0.10f, 0.11f, 0.13f, 1.0f));
                    const f32 knob_size = switch_size.y - 8.0f;
                    const f32 knob_travel = switch_size.x - knob_size - 8.0f;
                    draw_rect(context, RectF(0.0f, switch_size.y * -0.5f, switch_size.x, switch_size.y),
                        Float4U(0.0f, 0.5f, 0.0f, 0.0f), mix_color(off, on, selected), switch_size.y * 0.5f);
                    draw_rect(context, RectF(4.0f + knob_travel * selected, knob_size * -0.5f,
                        knob_size, knob_size), Float4U(0.0f, 0.5f, 0.0f, 0.0f), knob, knob_size * 0.5f);
                }
                GUICore::DrawCommand text;
                text.type = GUICore::DrawCommandType::text;
                text.rect_reference = GUICore::DrawCommandRectReference::element;
                text.rect = RectF(label_offset, 0.0f, -label_offset, 0.0f);
                text.text = data->label ? data->label : "";
                text.font = style_name(context, element, "gui.font");
                text.font_size = style_scalar(context, element, "gui.text.font_size", 16.0f);
                text.color = text_color;
                text.horizontal_alignment = VG::TextAlignment::begin;
                text.vertical_alignment = VG::TextAlignment::center;
                context->draw(text);
                return ok;
            }

            bool resolve_choice_action(GUICore::IContext* context, ChoiceAction& action)
            {
                bool changed = false;
                if(action.enabled && context->get_interaction_state(action.id).clicked)
                {
                    switch(action.operation)
                    {
                    case ChoiceOperation::toggle_bool:
                        if(action.bool_value) { *action.bool_value = !*action.bool_value; changed = true; }
                        break;
                    case ChoiceOperation::set_bool:
                        if(action.bool_value && !*action.bool_value) { *action.bool_value = true; changed = true; }
                        break;
                    case ChoiceOperation::set_int:
                        if(action.int_value && *action.int_value != action.set_value)
                        { *action.int_value = action.set_value; changed = true; }
                        break;
                    default: break;
                    }
                }
                bool selected = action.selected;
                if(action.operation == ChoiceOperation::toggle_bool || action.operation == ChoiceOperation::set_bool)
                    selected = action.bool_value ? *action.bool_value : selected;
                else if(action.operation == ChoiceOperation::set_int)
                    selected = action.int_value ? *action.int_value == action.set_value : selected;
                if(action.state)
                {
                    if(!action.state->initialized)
                    {
                        action.state->selected = selected ? 1.0f : 0.0f;
                        action.state->initialized = true;
                    }
                    f32 dt = max(context->get_frame_desc().delta_time, 0.0f);
                    GUICore::InteractionState interaction = context->get_interaction_state(action.id);
                    action.state->hovered = smooth_step(action.state->hovered,
                        action.enabled && interaction.hovered ? 1.0f : 0.0f, 12.0f, dt);
                    action.state->active = smooth_step(action.state->active,
                        action.enabled && interaction.active ? 1.0f : 0.0f, 18.0f, dt);
                    action.state->selected = smooth_step(action.state->selected, selected ? 1.0f : 0.0f, 14.0f, dt);
                }
                return changed;
            }

            static GUICore::ElementHandle choice(GUICore::IContext* context, id_t id, const c8* label,
                bool selected, ChoiceKind kind, const GUICore::LayoutConfig& layout, const ChoiceDesc& desc,
                ChoiceOperation operation, bool* bool_value, i32* int_value, i32 set_value)
            {
                GUICore::ElementHandle element = begin_element(context, id, label ? label : "Choice", layout);
                set_interactable(context, element, desc.enabled);
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.choice");
                callbacks.measure_callback = measure_choice;
                context->set_layout_callback_config(element, callbacks);
                Ref<ChoiceVisualState> state = widget_state<ChoiceVisualState>(context, id);
                ChoiceData* data = allocate_frame<ChoiceData>(context);
                data->label = copy_frame_string(context, label);
                data->kind = kind;
                data->enabled = desc.enabled;
                data->state = state.get();
                GUICore::DrawConfig draw;
                draw.name = Name("gui.choice");
                draw.callback = draw_choice;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                context->end_element();
                ChoiceAction* action = allocate_frame<ChoiceAction>(context);
                action->id = id;
                action->operation = operation;
                action->bool_value = bool_value;
                action->int_value = int_value;
                action->set_value = set_value;
                action->enabled = desc.enabled;
                action->selected = selected;
                action->state = state.get();
                add_action(context, ActionType::choice, id, action);
                return element;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle selectable(GUICore::IContext* context, id_t id, const c8* label,
            bool selected, const GUICore::LayoutConfig& layout, const ChoiceDesc& desc)
        {
            return Internal::choice(context, id, label, selected, Internal::ChoiceKind::selectable, layout, desc,
                Internal::ChoiceOperation::none, nullptr, nullptr, 0);
        }

        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, id_t id, const c8* label,
            bool* value, const GUICore::LayoutConfig& layout, const ChoiceDesc& desc)
        {
            return Internal::choice(context, id, label, value ? *value : false, Internal::ChoiceKind::checkbox,
                layout, desc, Internal::ChoiceOperation::toggle_bool, value, nullptr, 0);
        }

        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, id_t id, const c8* label,
            i32* value, i32 button_value, const GUICore::LayoutConfig& layout, const ChoiceDesc& desc)
        {
            return Internal::choice(context, id, label, value ? *value == button_value : false,
                Internal::ChoiceKind::radio, layout, desc, Internal::ChoiceOperation::set_int,
                nullptr, value, button_value);
        }

        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, id_t id, const c8* label,
            bool* value, const GUICore::LayoutConfig& layout, const ChoiceDesc& desc)
        {
            return Internal::choice(context, id, label, value ? *value : false, Internal::ChoiceKind::toggle,
                layout, desc, Internal::ChoiceOperation::toggle_bool, value, nullptr, 0);
        }
    }
}
