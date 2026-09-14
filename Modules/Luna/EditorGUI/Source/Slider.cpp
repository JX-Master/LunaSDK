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
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cmath>

namespace Luna
{
    namespace EditorGUI
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

            static GUI::MeasureResult measure_slider(GUI::IContext* context,
                const GUI::ElementHandle& element,
                const Float2U&, void*)
            {
                f32 height = style_scalar(context, element, "gui.control.height", 22.0f);
                GUI::MeasureResult result;
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

            static void draw_rounded_rect(GUI::IContext* context, const RectF& rect,
                const Float4U& scale, const Float4U& color, f32 radius,
                GUI::paint_order_id_t paint_order_id)
            {
                GUI::DrawCommand command;
                command.type = GUI::DrawCommandType::rounded_rect;
                command.rect_reference = GUI::DrawCommandRectReference::element;
                command.rect = rect;
                command.rect_layout_scale = scale;
                command.color = color;
                command.radius = radius;
                context->draw(command, paint_order_id);
            }

            static R<GUI::paint_order_id_t> draw_slider(GUI::IContext* context,
                const GUI::ElementHandle& element, GUI::DrawPhase,
                GUI::paint_order_id_t paint_order_id, void* userdata)
            {
                SliderData* data = (SliderData*)userdata;
                if(!data) return paint_order_id;
                f32 fraction = slider_fraction(*data);
                f32 knob_size = style_scalar(context, element, "gui.slider.knob_size", 12.0f);
                Float4U disabled = style_color(context, element, "gui.slider.disabled",
                    Float4U(0.32f, 0.36f, 0.42f, 1.0f));
                const RectF track_rect(knob_size * 0.5f, -2.0f, -knob_size, 4.0f);
                const Float4U track_scale(0.0f, 0.5f, 0.0f, 0.0f);
                draw_rounded_rect(context, track_rect, track_scale,
                    data->enabled ? style_color(context, element, "gui.slider.track",
                        Float4U(0.07f, 0.09f, 0.12f, 1.0f)) : disabled, 2.0f,
                    paint_order_id);
                if(fraction > 0.0f)
                {
                    const RectF fill_rect(knob_size * 0.5f, -2.0f, -knob_size * fraction, 4.0f);
                    const Float4U fill_scale(0.0f, 0.5f, fraction, 0.0f);
                    RoundedRectEffect fill_effects[2];
                    fill_effects[0].color = data->enabled ? style_color(context, element,
                        "gui.slider.fill", Float4U(0.20f, 0.42f, 0.72f, 1.0f)) : disabled;
                    u32 num_fill_effects = 1;
                    if(data->enabled)
                    {
                        Float4U highlight = style_color(context, element, "gui.shadow.inset_light",
                            Float4U(1.0f, 1.0f, 1.0f, 0.5f));
                        highlight.w *= 0.35f;
                        RoundedRectEffect& shadow = fill_effects[num_fill_effects++];
                        shadow.shadow = true;
                        shadow.color = highlight;
                        shadow.shadow_desc.offset = Float2U(0.0f, -1.0f);
                        shadow.shadow_desc.softness = 1.0f;
                        shadow.shadow_desc.mode = GUI::ShadowMode::inner;
                    }
                    if(RV result = draw_rounded_rect_effects(context, element, fill_rect, fill_scale,
                        2.0f, Span<const RoundedRectEffect>(fill_effects, num_fill_effects),
                        paint_order_id + 1); failed(result))
                    {
                        return result.errcode();
                    }
                }
                if(data->enabled)
                {
                    const f32 softness = style_scalar(context, element, "gui.shadow.softness", 5.0f);
                    RoundedRectEffect track_effects[2];
                    track_effects[0].shadow = true;
                    track_effects[0].color = style_color(context, element, "gui.shadow.inset",
                        Float4U(0.0f, 0.0f, 0.0f, 0.18f));
                    track_effects[0].shadow_desc.offset = Float2U(1.5f, 1.5f);
                    track_effects[0].shadow_desc.softness = softness * 0.45f;
                    track_effects[0].shadow_desc.mode = GUI::ShadowMode::inner;
                    track_effects[1].shadow = true;
                    track_effects[1].color = style_color(context, element, "gui.shadow.inset_light",
                        Float4U(1.0f, 1.0f, 1.0f, 0.65f));
                    track_effects[1].shadow_desc.offset = Float2U(-1.5f, -1.5f);
                    track_effects[1].shadow_desc.softness = softness * 0.35f;
                    track_effects[1].shadow_desc.mode = GUI::ShadowMode::inner;
                    if(RV result = draw_rounded_rect_effects(context, element, track_rect, track_scale,
                        2.0f, Span<const RoundedRectEffect>(track_effects, 2),
                        paint_order_id + 2); failed(result))
                    {
                        return result.errcode();
                    }
                }
                const RectF knob_rect(-knob_size * fraction, -knob_size * 0.5f, knob_size, knob_size);
                const Float4U knob_scale(fraction, 0.5f, 0.0f, 0.0f);
                RoundedRectEffect knob_effects[3];
                u32 num_knob_effects = 0;
                if(data->enabled)
                {
                    const Float2U shadow_offset = style_vector2(context, element, "gui.shadow.offset",
                        Float2U(2.0f));
                    const f32 softness = style_scalar(context, element, "gui.shadow.softness", 5.0f) * 0.55f;
                    RoundedRectEffect& dark = knob_effects[num_knob_effects++];
                    dark.shadow = true;
                    dark.color = style_color(context, element, "gui.shadow.dark",
                        Float4U(0.0f, 0.0f, 0.0f, 0.20f));
                    dark.shadow_desc.offset = shadow_offset;
                    dark.shadow_desc.softness = softness;
                    RoundedRectEffect& light = knob_effects[num_knob_effects++];
                    light.shadow = true;
                    light.color = style_color(context, element, "gui.shadow.light",
                        Float4U(1.0f, 1.0f, 1.0f, 0.75f));
                    light.shadow_desc.offset = Float2U(-shadow_offset.x, -shadow_offset.y);
                    light.shadow_desc.softness = softness;
                }
                knob_effects[num_knob_effects++].color = data->enabled ?
                    style_color(context, element, "gui.slider.knob",
                        Float4U(0.32f, 0.58f, 0.90f, 1.0f)) : disabled;
                if(RV result = draw_rounded_rect_effects(context, element, knob_rect, knob_scale,
                    knob_size * 0.5f, Span<const RoundedRectEffect>(knob_effects,
                        num_knob_effects), paint_order_id + 3); failed(result))
                {
                    return result.errcode();
                }
                return paint_order_id + 3;
            }

            static bool resolve_slider_position(GUI::IContext* context, id_t id, f32& fraction)
            {
                bool changed = false;
                for(const GUI::RoutedInputEvent& routed : context->get_routed_input_events(id))
                {
                    if(!routed.has_element_position) continue;
                    if((routed.event.type == GUI::InputEventType::pointer_down &&
                        routed.event.button == GUI::PointerButton::left) ||
                        (routed.event.type == GUI::InputEventType::pointer_move &&
                        context->is_pointer_button_down(GUI::PointerButton::left)))
                    {
                        const GUI::Element* element = context->find_element(id);
                        f32 width = element ? max(element->layout_result.rect.width, 1.0f) : 1.0f;
                        fraction = clamp(routed.element_position.x / width, 0.0f, 1.0f);
                        changed = true;
                    }
                }
                return changed;
            }

            bool resolve_slider_float_action(GUI::IContext* context, SliderFloatAction& action)
            {
                if(!action.enabled || !action.value) return false;
                f32 fraction = action.maximum > action.minimum ? (*action.value - action.minimum) /
                    (action.maximum - action.minimum) : 0.0f;
                bool changed = resolve_slider_position(context, action.id, fraction);
                for(const GUI::InputEvent& event : context->get_delivered_input_events(action.id))
                {
                    if(event.type == GUI::InputEventType::key_down &&
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

            bool resolve_slider_int_action(GUI::IContext* context, SliderIntAction& action)
            {
                if(!action.enabled || !action.value) return false;
                f32 fraction = action.maximum > action.minimum ? (f32)(*action.value - action.minimum) /
                    (f32)(action.maximum - action.minimum) : 0.0f;
                bool changed = resolve_slider_position(context, action.id, fraction);
                for(const GUI::InputEvent& event : context->get_delivered_input_events(action.id))
                {
                    if(event.type == GUI::InputEventType::key_down &&
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

            static GUI::ElementHandle slider_common(GUI::IContext* context, id_t id,
                const GUI::LayoutConfig& layout, SliderData* data)
            {
                GUI::ElementHandle element = begin_element(context, id, "Slider", layout);
                set_interactable(context, element, data->enabled);
                GUI::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.slider");
                callbacks.measure_callback = measure_slider;
                context->set_layout_callback_config(element, callbacks);
                GUI::DrawConfig draw;
                draw.name = Name("gui.slider");
                draw.callback = draw_slider;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                context->end_element();
                return element;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_float(GUI::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value);
            if(maximum < minimum) swap(minimum, maximum);
            *value = clamp(*value, minimum, maximum);
            Internal::SliderData* data = Internal::allocate_frame<Internal::SliderData>(context);
            data->float_value = value;
            data->minimum = minimum;
            data->maximum = maximum;
            data->enabled = desc.enabled;
            GUI::ElementHandle element = Internal::slider_common(context, id, layout, data);
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

        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_int(GUI::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value);
            if(maximum < minimum) swap(minimum, maximum);
            *value = clamp(*value, minimum, maximum);
            Internal::SliderData* data = Internal::allocate_frame<Internal::SliderData>(context);
            data->int_value = value;
            data->minimum = (f32)minimum;
            data->maximum = (f32)maximum;
            data->enabled = desc.enabled;
            GUI::ElementHandle element = Internal::slider_common(context, id, layout, data);
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

        static GUI::ElementHandle slider_float_n(GUI::IContext* context, id_t id, f32* value,
            u32 count, f32 minimum, f32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value && count >= 2 && count <= 4);
            GUI::ElementHandle group = begin_h_layout(context, id, "Vector Float Slider", layout);
            for(u32 i = 0; i < count; ++i)
            {
                GUI::LayoutConfig child_layout;
                child_layout.width.kind = GUI::SizeKind::fit;
                child_layout.width.min = 48.0f;
                child_layout.height.kind = GUI::SizeKind::percent;
                child_layout.height.value = 1.0f;
                child_layout.flex_grow = 1.0f;
                slider_float(context, GUI::make_scoped_id(id, (id_t)i + 1), value + i,
                    minimum, maximum, child_layout, desc);
            }
            GUI::FlexLayoutDesc flex;
            flex.main_axis_gap = 6.0f;
            end_h_layout(context, group, flex);
            return group;
        }

        static GUI::ElementHandle slider_int_n(GUI::IContext* context, id_t id, i32* value,
            u32 count, i32 minimum, i32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        {
            luassert(context && id && value && count >= 2 && count <= 4);
            GUI::ElementHandle group = begin_h_layout(context, id, "Vector Integer Slider", layout);
            for(u32 i = 0; i < count; ++i)
            {
                GUI::LayoutConfig child_layout;
                child_layout.width.kind = GUI::SizeKind::fit;
                child_layout.width.min = 48.0f;
                child_layout.height.kind = GUI::SizeKind::percent;
                child_layout.height.value = 1.0f;
                child_layout.flex_grow = 1.0f;
                slider_int(context, GUI::make_scoped_id(id, (id_t)i + 1), value + i,
                    minimum, maximum, child_layout, desc);
            }
            GUI::FlexLayoutDesc flex;
            flex.main_axis_gap = 6.0f;
            end_h_layout(context, group, flex);
            return group;
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_float2(GUI::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_float_n(context, id, value, 2, minimum, maximum, layout, desc); }
        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_float3(GUI::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_float_n(context, id, value, 3, minimum, maximum, layout, desc); }
        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_float4(GUI::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_float_n(context, id, value, 4, minimum, maximum, layout, desc); }
        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_int2(GUI::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_int_n(context, id, value, 2, minimum, maximum, layout, desc); }
        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_int3(GUI::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_int_n(context, id, value, 3, minimum, maximum, layout, desc); }
        LUNA_EDITOR_GUI_API GUI::ElementHandle slider_int4(GUI::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUI::LayoutConfig& layout, const SliderDesc& desc)
        { return slider_int_n(context, id, value, 4, minimum, maximum, layout, desc); }
    }
}
