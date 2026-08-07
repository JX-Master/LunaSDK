/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Drag.cpp
* @author JXMaster
* @date 2026/7/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <cmath>
#include <cstdio>

namespace Luna
{
    namespace GUI
    {
        namespace Internal
        {
            struct DragData
            {
                f32* float_value = nullptr;
                i32* int_value = nullptr;
                bool enabled = true;
            };

            static GUICore::MeasureResult measure_drag(GUICore::IContext* context,
                const GUICore::ElementHandle& element,
                const Float2U&, void*)
            {
                f32 height = style_scalar(context, element, "gui.control.height", 28.0f);
                GUICore::MeasureResult result;
                result.minimum = Float2U(48.0f, height);
                result.desired = Float2U(120.0f, height);
                return result;
            }

            static RV draw_drag(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                DragData* data = (DragData*)userdata;
                if(!data) return ok;
                GUICore::InteractionState interaction = context->get_interaction_state(element.id);
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.color = style_color(context, element, !data->enabled ? "gui.drag.disabled" :
                    (interaction.active ? "gui.drag.active" : (interaction.hovered ? "gui.drag.hovered" :
                    "gui.drag.background")), !data->enabled ? Float4U(0.09f, 0.11f, 0.14f, 1.0f) :
                    (interaction.active ? Float4U(0.18f, 0.32f, 0.50f, 1.0f) :
                    (interaction.hovered ? Float4U(0.14f, 0.21f, 0.30f, 1.0f) :
                    Float4U(0.11f, 0.15f, 0.21f, 1.0f))));
                command.radius = 4.0f;
                context->draw(command);
                c8 value[64];
                if(data->float_value) snprintf(value, sizeof(value), "%.3f", *data->float_value);
                else snprintf(value, sizeof(value), "%d", data->int_value ? *data->int_value : 0);
                command.type = GUICore::DrawCommandType::text;
                command.rect = RectF(8.0f, 0.0f, -16.0f, 0.0f);
                command.text = value;
                command.font = style_name(context, element, "gui.font");
                command.font_size = style_scalar(context, element, "gui.text.font_size", 15.0f);
                command.color = data->enabled ? style_color(context, element, "gui.text.color",
                    Float4U(0.86f, 0.88f, 0.92f, 1.0f)) : style_color(context, element,
                    "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
                command.horizontal_alignment = VG::TextAlignment::begin;
                command.vertical_alignment = VG::TextAlignment::center;
                context->draw(command);
                return ok;
            }

            template <typename T>
            static bool resolve_drag(GUICore::IContext* context, id_t id, T* value, T minimum, T maximum,
                f32 speed, bool enabled, DragState* state)
            {
                if(!enabled || !value || !state) return false;
                bool changed = false;
                for(const GUICore::RoutedInputEvent& routed : context->get_routed_input_events(id))
                {
                    if(!routed.has_element_position) continue;
                    if(routed.event.type == GUICore::InputEventType::pointer_down &&
                        routed.event.button == GUICore::PointerButton::left)
                    {
                        state->dragging = true;
                        state->start_pointer_x = routed.element_position.x;
                        if constexpr(is_same_v<T, f32>) state->start_float = *value;
                        else state->start_int = *value;
                    }
                    else if(routed.event.type == GUICore::InputEventType::pointer_move && state->dragging)
                    {
                        f32 start = is_same_v<T, f32> ? state->start_float : (f32)state->start_int;
                        f32 next = start + (routed.element_position.x - state->start_pointer_x) * speed;
                        T converted;
                        if constexpr(is_same_v<T, f32>) converted = next;
                        else converted = (T)round(next);
                        if(maximum > minimum) converted = clamp(converted, minimum, maximum);
                        if(*value != converted) { *value = converted; changed = true; }
                    }
                    else if(routed.event.type == GUICore::InputEventType::pointer_up &&
                        routed.event.button == GUICore::PointerButton::left)
                    {
                        state->dragging = false;
                    }
                }
                return changed;
            }

            bool resolve_drag_float_action(GUICore::IContext* context, DragFloatAction& action)
            {
                return resolve_drag(context, action.id, action.value, action.minimum, action.maximum,
                    action.speed, action.enabled, action.state);
            }

            bool resolve_drag_int_action(GUICore::IContext* context, DragIntAction& action)
            {
                return resolve_drag(context, action.id, action.value, action.minimum, action.maximum,
                    action.speed, action.enabled, action.state);
            }

            static GUICore::ElementHandle drag_float_scalar(GUICore::IContext* context, id_t id, f32* value,
                f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const DragDesc& desc)
            {
                GUICore::ElementHandle element = begin_element(context, id, "Float Drag", layout);
                set_interactable(context, element, desc.enabled);
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.drag");
                callbacks.measure_callback = measure_drag;
                context->set_layout_callback_config(element, callbacks);
                DragData* data = allocate_frame<DragData>(context);
                data->float_value = value;
                data->enabled = desc.enabled;
                GUICore::DrawConfig draw;
                draw.name = Name("gui.drag");
                draw.callback = draw_drag;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                context->end_element();
                Ref<DragState> state = widget_state<DragState>(context, id);
                DragFloatAction* action = allocate_frame<DragFloatAction>(context);
                action->id = id; action->value = value; action->minimum = minimum; action->maximum = maximum;
                action->speed = desc.speed; action->enabled = desc.enabled; action->state = state.get();
                add_action(context, ActionType::drag_float, id, action);
                return element;
            }

            static GUICore::ElementHandle drag_int_scalar(GUICore::IContext* context, id_t id, i32* value,
                i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const DragDesc& desc)
            {
                GUICore::ElementHandle element = begin_element(context, id, "Integer Drag", layout);
                set_interactable(context, element, desc.enabled);
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.drag");
                callbacks.measure_callback = measure_drag;
                context->set_layout_callback_config(element, callbacks);
                DragData* data = allocate_frame<DragData>(context);
                data->int_value = value;
                data->enabled = desc.enabled;
                GUICore::DrawConfig draw;
                draw.name = Name("gui.drag");
                draw.callback = draw_drag;
                draw.userdata = data;
                context->set_draw_config(element, draw);
                context->end_element();
                Ref<DragState> state = widget_state<DragState>(context, id);
                DragIntAction* action = allocate_frame<DragIntAction>(context);
                action->id = id; action->value = value; action->minimum = minimum; action->maximum = maximum;
                action->speed = desc.speed; action->enabled = desc.enabled; action->state = state.get();
                add_action(context, ActionType::drag_int, id, action);
                return element;
            }

            template <typename T, typename Function>
            static GUICore::ElementHandle drag_vector(GUICore::IContext* context, id_t id, T* value, u32 count,
                const GUICore::LayoutConfig& layout, Function&& function)
            {
                GUICore::ElementHandle group = begin_h_layout(context, id, "Vector Drag", layout);
                for(u32 i = 0; i < count; ++i)
                {
                    GUICore::LayoutConfig child;
                    child.width.kind = GUICore::SizeKind::fit;
                    child.width.min = 48.0f;
                    child.height.kind = GUICore::SizeKind::percent;
                    child.height.value = 1.0f;
                    child.flex_grow = 1.0f;
                    function(GUICore::make_scoped_id(id, (id_t)i + 1), value + i, child);
                }
                GUICore::FlexLayoutDesc flex;
                flex.main_axis_gap = 6.0f;
                end_h_layout(context, group, flex);
                return group;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle drag_float(GUICore::IContext* context, id_t id, f32* value,
            f32 minimum, f32 maximum, const GUICore::LayoutConfig& layout, const DragDesc& desc)
        { return Internal::drag_float_scalar(context, id, value, minimum, maximum, layout, desc); }
        LUNA_GUI_API GUICore::ElementHandle drag_int(GUICore::IContext* context, id_t id, i32* value,
            i32 minimum, i32 maximum, const GUICore::LayoutConfig& layout, const DragDesc& desc)
        { return Internal::drag_int_scalar(context, id, value, minimum, maximum, layout, desc); }

#define LUNA_GUI_DEFINE_VECTOR_DRAG(TYPE, SUFFIX, COUNT, SCALAR) \
        LUNA_GUI_API GUICore::ElementHandle drag_##SUFFIX##COUNT(GUICore::IContext* context, id_t id, TYPE* value, \
            TYPE minimum, TYPE maximum, const GUICore::LayoutConfig& layout, const DragDesc& desc) \
        { \
            return Internal::drag_vector(context, id, value, COUNT, layout, [&](id_t child_id, TYPE* child_value, \
                const GUICore::LayoutConfig& child_layout) { SCALAR(context, child_id, child_value, minimum, maximum, \
                child_layout, desc); }); \
        }
        LUNA_GUI_DEFINE_VECTOR_DRAG(f32, float, 2, Internal::drag_float_scalar)
        LUNA_GUI_DEFINE_VECTOR_DRAG(f32, float, 3, Internal::drag_float_scalar)
        LUNA_GUI_DEFINE_VECTOR_DRAG(f32, float, 4, Internal::drag_float_scalar)
        LUNA_GUI_DEFINE_VECTOR_DRAG(i32, int, 2, Internal::drag_int_scalar)
        LUNA_GUI_DEFINE_VECTOR_DRAG(i32, int, 3, Internal::drag_int_scalar)
        LUNA_GUI_DEFINE_VECTOR_DRAG(i32, int, 4, Internal::drag_int_scalar)
#undef LUNA_GUI_DEFINE_VECTOR_DRAG
    }
}
