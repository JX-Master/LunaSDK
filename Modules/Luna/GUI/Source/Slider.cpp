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

            static GUICore::MeasureResult measure_slider(GUICore::IContext* context,
                const GUICore::ElementHandle& element,
                const Float2U&, void*)
            {
                f32 height = style_scalar(context, element, "gui.control.height", 22.0f);
                GUICore::MeasureResult result;
                result.minimum = Float2U(48.0f, height);
                result.desired = Float2U(160.0f, height);
                return result;
            }

            static f32 slider_fraction(const SliderData& data)
            {
                f32 value = data.float_value ? *data.float_value : (f32)*data.int_value;
                return data.maximum > data.minimum ? clamp((value - data.minimum) / (data.maximum - data.minimum),
                    0.0f, 1.0f) : 0.0f;
            }

            static void draw_rounded_rect(GUICore::IContext* context, const RectF& rect,
                const Float4U& scale, const Float4U& color, f32 radius)
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

            static void draw_shadow(GUICore::IContext* context, const RectF& rect,
                const Float4U& scale, const Float4U& color, f32 radius, const Float2U& offset,
                f32 softness, GUICore::ShadowMode mode = GUICore::ShadowMode::outer)
            {
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::shadow;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.rect = rect;
                command.rect_layout_scale = scale;
                command.color = color;
                command.radius = radius;
                command.shadow.offset = offset;
                command.shadow.softness = softness;
                command.shadow.mode = mode;
                context->draw(command);
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
                const RectF track_rect(knob_size * 0.5f, -2.0f, -knob_size, 4.0f);
                const Float4U track_scale(0.0f, 0.5f, 0.0f, 0.0f);
                draw_rounded_rect(context, track_rect, track_scale,
                    data->enabled ? style_color(context, element, "gui.slider.track",
                        Float4U(0.07f, 0.09f, 0.12f, 1.0f)) : disabled, 2.0f);
                if(fraction > 0.0f)
                {
                    const RectF fill_rect(knob_size * 0.5f, -2.0f, -knob_size * fraction, 4.0f);
                    const Float4U fill_scale(0.0f, 0.5f, fraction, 0.0f);
                    draw_rounded_rect(context, fill_rect, fill_scale,
                        data->enabled ? style_color(context, element, "gui.slider.fill",
                            Float4U(0.20f, 0.42f, 0.72f, 1.0f)) : disabled, 2.0f);
                    if(data->enabled)
                    {
                        Float4U highlight = style_color(context, element, "gui.shadow.inset_light",
                            Float4U(1.0f, 1.0f, 1.0f, 0.5f));
                        highlight.w *= 0.35f;
                        draw_shadow(context, fill_rect, fill_scale, highlight, 2.0f,
                            Float2U(0.0f, -1.0f), 1.0f, GUICore::ShadowMode::inner);
                    }
                }
                if(data->enabled)
                {
                    const f32 softness = style_scalar(context, element, "gui.shadow.softness", 5.0f);
                    draw_shadow(context, track_rect, track_scale,
                        style_color(context, element, "gui.shadow.inset",
                            Float4U(0.0f, 0.0f, 0.0f, 0.18f)), 2.0f,
                        Float2U(1.5f, 1.5f), softness * 0.45f, GUICore::ShadowMode::inner);
                    draw_shadow(context, track_rect, track_scale,
                        style_color(context, element, "gui.shadow.inset_light",
                            Float4U(1.0f, 1.0f, 1.0f, 0.65f)), 2.0f,
                        Float2U(-1.5f, -1.5f), softness * 0.35f, GUICore::ShadowMode::inner);
                }
                const RectF knob_rect(-knob_size * fraction, -knob_size * 0.5f, knob_size, knob_size);
                const Float4U knob_scale(fraction, 0.5f, 0.0f, 0.0f);
                if(data->enabled)
                {
                    const Float2U shadow_offset = style_vector2(context, element, "gui.shadow.offset",
                        Float2U(2.0f));
                    const f32 softness = style_scalar(context, element, "gui.shadow.softness", 5.0f) * 0.55f;
                    draw_shadow(context, knob_rect, knob_scale,
                        style_color(context, element, "gui.shadow.dark",
                            Float4U(0.0f, 0.0f, 0.0f, 0.20f)), knob_size * 0.5f,
                        shadow_offset, softness);
                    draw_shadow(context, knob_rect, knob_scale,
                        style_color(context, element, "gui.shadow.light",
                            Float4U(1.0f, 1.0f, 1.0f, 0.75f)), knob_size * 0.5f,
                        Float2U(-shadow_offset.x, -shadow_offset.y), softness);
                }
                draw_rounded_rect(context, knob_rect, knob_scale,
                    data->enabled ? style_color(context, element, "gui.slider.knob",
                        Float4U(0.32f, 0.58f, 0.90f, 1.0f)) : disabled, knob_size * 0.5f);
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

        static GUICore::ElementHandle slider_float_n(GUICore::IContext* context, id_t id, f32* value,
            u32 count, f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value && count >= 2 && count <= 4);
            GUICore::ElementHandle group = begin_h_layout(context, id, "Vector Float Slider", layout);
            for(u32 i = 0; i < count; ++i)
            {
                GUICore::LayoutConfig child_layout;
                child_layout.width.kind = GUICore::SizeKind::fit;
                child_layout.width.min = 48.0f;
                child_layout.height.kind = GUICore::SizeKind::percent;
                child_layout.height.value = 1.0f;
                child_layout.flex_grow = 1.0f;
                slider_float(context, GUICore::make_scoped_id(id, (id_t)i + 1), value + i,
                    minimum, maximum, child_layout, desc);
            }
            GUICore::FlexLayoutDesc flex;
            flex.main_axis_gap = 6.0f;
            end_h_layout(context, group, flex);
            return group;
        }

        static GUICore::ElementHandle slider_int_n(GUICore::IContext* context, id_t id, i32* value,
            u32 count, i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value && count >= 2 && count <= 4);
            GUICore::ElementHandle group = begin_h_layout(context, id, "Vector Integer Slider", layout);
            for(u32 i = 0; i < count; ++i)
            {
                GUICore::LayoutConfig child_layout;
                child_layout.width.kind = GUICore::SizeKind::fit;
                child_layout.width.min = 48.0f;
                child_layout.height.kind = GUICore::SizeKind::percent;
                child_layout.height.value = 1.0f;
                child_layout.flex_grow = 1.0f;
                slider_int(context, GUICore::make_scoped_id(id, (id_t)i + 1), value + i,
                    minimum, maximum, child_layout, desc);
            }
            GUICore::FlexLayoutDesc flex;
            flex.main_axis_gap = 6.0f;
            end_h_layout(context, group, flex);
            return group;
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float2(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_float_n(context, id, value, 2, minimum, maximum, layout, desc); }
        LUNA_GUI_API GUICore::ElementHandle slider_float3(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_float_n(context, id, value, 3, minimum, maximum, layout, desc); }
        LUNA_GUI_API GUICore::ElementHandle slider_float4(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_float_n(context, id, value, 4, minimum, maximum, layout, desc); }
        LUNA_GUI_API GUICore::ElementHandle slider_int2(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_int_n(context, id, value, 2, minimum, maximum, layout, desc); }
        LUNA_GUI_API GUICore::ElementHandle slider_int3(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_int_n(context, id, value, 3, minimum, maximum, layout, desc); }
        LUNA_GUI_API GUICore::ElementHandle slider_int4(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_int_n(context, id, value, 4, minimum, maximum, layout, desc); }
    }
}
