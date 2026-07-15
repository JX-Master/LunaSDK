/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Slider.cpp
* @author JXMaster
* @date 2026/7/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cmath>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct SliderData
            {
                f32* float_value = nullptr;
                i32* int_value = nullptr;
                f32 minimum = 0.0f;
                f32 maximum = 1.0f;
                bool enabled = true;
            };

            static GUICore::MeasureResult measure_slider(GUICore::IContext*, const GUICore::ElementHandle&,
                const Float2U&, void*)
            {
                GUICore::MeasureResult result;
                result.minimum = Float2U(48.0f, 18.0f);
                result.desired = Float2U(160.0f, 22.0f);
                return result;
            }

            static f32 slider_fraction(const SliderData& data)
            {
                f32 value = data.float_value ? *data.float_value : (f32)*data.int_value;
                return data.maximum > data.minimum ? clamp((value - data.minimum) / (data.maximum - data.minimum),
                    0.0f, 1.0f) : 0.0f;
            }

            static RV draw_slider(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                SliderData* data = (SliderData*)userdata;
                if(!data) return ok;
                f32 fraction = slider_fraction(*data);
                f32 knob_size = style_scalar(context, element, "gui.slider.knob_size", 12.0f);
                Float4U disabled = style_color(context, element, "gui.slider.disabled",
                    Float4U(0.32f, 0.36f, 0.42f, 1.0f));
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = RectF(knob_size * 0.5f, -2.0f, -knob_size, 4.0f);
                command.rect_layout_scale = Float4U(0.0f, 0.5f, 0.0f, 0.0f);
                command.color = data->enabled ? style_color(context, element, "gui.slider.track",
                    Float4U(0.07f, 0.09f, 0.12f, 1.0f)) : disabled;
                command.radius = 2.0f;
                context->draw(command);
                if(fraction > 0.0f)
                {
                    command.rect = RectF(knob_size * 0.5f, -2.0f, -knob_size * 0.5f, 4.0f);
                    command.rect_layout_scale = Float4U(0.0f, 0.5f, fraction, 0.0f);
                    command.color = data->enabled ? style_color(context, element, "gui.slider.fill",
                        Float4U(0.20f, 0.42f, 0.72f, 1.0f)) : disabled;
                    context->draw(command);
                }
                command.rect = RectF(-knob_size * 0.5f, -knob_size * 0.5f, knob_size, knob_size);
                command.rect_layout_scale = Float4U(fraction, 0.5f, 0.0f, 0.0f);
                command.color = data->enabled ? style_color(context, element, "gui.slider.knob",
                    Float4U(0.32f, 0.58f, 0.90f, 1.0f)) : disabled;
                command.radius = knob_size * 0.5f;
                context->draw(command);
                return ok;
            }

            static bool resolve_slider_position(GUICore::IContext* context, id_t id, f32& fraction)
            {
                bool changed = false;
                for(const GUICore::RoutedInputEvent& routed : context->get_routed_input_events(id))
                {
                    if(!routed.has_element_position) continue;
                    if((routed.event.type == GUICore::InputEventType::pointer_down &&
                        routed.event.button == GUICore::PointerButton::left) ||
                        (routed.event.type == GUICore::InputEventType::pointer_move &&
                        context->is_pointer_button_down(GUICore::PointerButton::left)))
                    {
                        const GUICore::Element* element = context->find_element(id);
                        f32 width = element ? max(element->layout_result.rect.width, 1.0f) : 1.0f;
                        fraction = clamp(routed.element_position.x / width, 0.0f, 1.0f);
                        changed = true;
                    }
                }
                return changed;
            }

            bool resolve_slider_float_action(GUICore::IContext* context, SliderFloatAction& action)
            {
                if(!action.enabled || !action.value) return false;
                f32 fraction = action.maximum > action.minimum ? (*action.value - action.minimum) /
                    (action.maximum - action.minimum) : 0.0f;
                bool changed = resolve_slider_position(context, action.id, fraction);
                for(const GUICore::InputEvent& event : context->get_delivered_input_events(action.id))
                {
                    if(event.type == GUICore::InputEventType::key_down &&
                        (event.key == KeyCode::left || event.key == KeyCode::right))
                    {
                        fraction += event.key == KeyCode::left ? -action.navigation_step : action.navigation_step;
                        changed = true;
                    }
                }
                if(changed)
                {
                    *action.value = action.minimum + clamp(fraction, 0.0f, 1.0f) * (action.maximum - action.minimum);
                }
                return changed;
            }

            bool resolve_slider_int_action(GUICore::IContext* context, SliderIntAction& action)
            {
                if(!action.enabled || !action.value) return false;
                f32 fraction = action.maximum > action.minimum ? (f32)(*action.value - action.minimum) /
                    (f32)(action.maximum - action.minimum) : 0.0f;
                bool changed = resolve_slider_position(context, action.id, fraction);
                for(const GUICore::InputEvent& event : context->get_delivered_input_events(action.id))
                {
                    if(event.type == GUICore::InputEventType::key_down &&
                        (event.key == KeyCode::left || event.key == KeyCode::right))
                    {
                        f32 step = max(action.navigation_step, 1.0f / max((f32)(action.maximum - action.minimum), 1.0f));
                        fraction += event.key == KeyCode::left ? -step : step;
                        changed = true;
                    }
                }
                if(changed)
                {
                    *action.value = clamp((i32)round(action.minimum + clamp(fraction, 0.0f, 1.0f) *
                        (f32)(action.maximum - action.minimum)), action.minimum, action.maximum);
                }
                return changed;
            }

            static GUICore::ElementHandle slider_common(GUICore::IContext* context, id_t id,
                const GUICore::LayoutConfig& layout, SliderData* data)
            {
                GUICore::ElementHandle element = begin_element(context, id, "Slider", layout);
                set_interactable(context, element, data->enabled);
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.slider");
                callbacks.measure_callback = measure_slider;
                context->set_layout_callback_config(element, callbacks);
                GUICore::DrawConfig draw;
                draw.name = Name("gui.slider");
                draw.callback = draw_slider;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                context->end_element();
                return element;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value);
            if(maximum < minimum) swap(minimum, maximum);
            *value = clamp(*value, minimum, maximum);
            Internal::SliderData* data = Internal::allocate_frame<Internal::SliderData>(context);
            data->float_value = value;
            data->minimum = minimum;
            data->maximum = maximum;
            data->enabled = desc.enabled;
            GUICore::ElementHandle element = Internal::slider_common(context, id, layout, data);
            Internal::SliderFloatAction* action = Internal::allocate_frame<Internal::SliderFloatAction>(context);
            action->id = id;
            action->value = value;
            action->minimum = minimum;
            action->maximum = maximum;
            action->navigation_step = desc.navigation_step;
            action->enabled = desc.enabled;
            Internal::add_action(context, Internal::ActionType::slider_float, id, action);
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value);
            if(maximum < minimum) swap(minimum, maximum);
            *value = clamp(*value, minimum, maximum);
            Internal::SliderData* data = Internal::allocate_frame<Internal::SliderData>(context);
            data->int_value = value;
            data->minimum = (f32)minimum;
            data->maximum = (f32)maximum;
            data->enabled = desc.enabled;
            GUICore::ElementHandle element = Internal::slider_common(context, id, layout, data);
            Internal::SliderIntAction* action = Internal::allocate_frame<Internal::SliderIntAction>(context);
            action->id = id;
            action->value = value;
            action->minimum = minimum;
            action->maximum = maximum;
            action->navigation_step = desc.navigation_step;
            action->enabled = desc.enabled;
            Internal::add_action(context, Internal::ActionType::slider_int, id, action);
            return element;
        }
    }
}
