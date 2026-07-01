/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorNumeric.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorState.hpp>
#include <Luna/GUI/EditorWidgets.hpp>

namespace Luna
{
    namespace GUI
    {
        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry,
            const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static void set_basic_interactable(GUICore::IContext* context, const GUICore::ElementHandle& element)
        {
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
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
            const c8* text, const Float4U& color, f32 font_size)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
            command.rect_layout_scale = scale;
            command.color = color;
            command.font_size = font_size;
            command.horizontal_alignment = VG::TextAlignment::begin;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
        }

        static f32 slider_fraction(f32 value, f32 min_value, f32 max_value)
        {
            if(max_value == min_value)
            {
                return 0.0f;
            }
            return clamp((value - min_value) / (max_value - min_value), 0.0f, 1.0f);
        }

        static bool slider_interaction_fraction(GUICore::IContext* context, GUICore::id_t id,
            u8 component_count, u8& component_index, f32& fraction)
        {
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            RectF rect = interaction.active ? interaction.pointer_element_rect : interaction.clicked_element_rect;
            Float2U position = interaction.active ? interaction.pointer_element_position : interaction.clicked_element_position;
            if((interaction.active || interaction.clicked) && rect.width > 0.0f && rect.height > 0.0f && component_count)
            {
                f32 row_height = rect.height / (f32)component_count;
                i32 row_index = (i32)(position.y / max(row_height, 1.0f));
                component_index = (u8)clamp(row_index, 0, (i32)component_count - 1);
                fraction = clamp(position.x / rect.width, 0.0f, 1.0f);
                return true;
            }
            return false;
        }

        static GUICore::ElementHandle slider(GUICore::IContext* context, GUICore::id_t id, Span<const f32> fractions,
            const GUICore::LayoutConfig& layout, const Name& debug_name)
        {
            GUICore::ElementHandle element = context->begin_element(id, debug_name);
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element);

            f32 track_width = style_value(context, Name("gui.editor.slider.track_width"), GUICore::style_f32(2.0f)).number.x;
            f32 fill_width = style_value(context, Name("gui.editor.slider.fill_width"), GUICore::style_f32(2.0f)).number.x;
            f32 knob_size = style_value(context, Name("gui.editor.slider.knob_size"), GUICore::style_f32(12.0f)).number.x;
            Float4U track_color = style_value(context, Name("gui.editor.slider.track"),
                GUICore::style_f32x4(Float4U(0.07f, 0.09f, 0.12f, 1.0f))).number;
            Float4U fill_color = style_value(context, Name("gui.editor.slider.fill"),
                GUICore::style_f32x4(Float4U(0.20f, 0.42f, 0.72f, 1.0f))).number;
            Float4U knob_color = style_value(context, Name("gui.editor.slider.knob"),
                GUICore::style_f32x4(Float4U(0.32f, 0.55f, 0.88f, 1.0f))).number;
            f32 inset = knob_size * 0.5f;
            u32 count = max((u32)fractions.size(), 1u);
            for(u32 i = 0; i < count; ++i)
            {
                f32 t = clamp(fractions.empty() ? 0.0f : fractions[i], 0.0f, 1.0f);
                f32 row_center = ((f32)i + 0.5f) / (f32)count;
                Float4U full_scale(0.0f, row_center, 1.0f, row_center);
                Float4U fill_scale(0.0f, row_center, t, row_center);
                Float4U knob_scale(t, row_center, 0.0f, 0.0f);
                draw_relative_line(context, Float2U(inset, 0.0f), Float2U(-inset, 0.0f),
                    track_color, track_width, full_scale);
                draw_relative_line(context, Float2U(inset, 0.0f), Float2U(0.0f, 0.0f),
                    fill_color, fill_width, fill_scale);
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(-inset, -inset, knob_size, knob_size), knob_color, inset, knob_scale);
            }
            context->end_element();
            return element;
        }

        static GUICore::ElementHandle slider_float_n(GUICore::IContext* context, GUICore::id_t id, f32* value,
            u8 count, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            Vector<f32> fractions(count);
            for(u32 i = 0; i < count; ++i)
            {
                fractions[i] = value ? slider_fraction(value[i], min_value, max_value) : 0.0f;
            }
            f32 interaction_fraction = 0.0f;
            u8 component_index = 0;
            if(value && max_value != min_value && slider_interaction_fraction(context, id, count, component_index, interaction_fraction))
            {
                value[component_index] = min_value + (max_value - min_value) * interaction_fraction;
                fractions[component_index] = interaction_fraction;
            }
            return slider(context, id, fractions.cspan(), layout, count == 1 ? Name("slider_float") : Name("slider_float_n"));
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_float_n(context, id, value, 1, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float2(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_float_n(context, id, value, 2, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float3(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_float_n(context, id, value, 3, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_float4(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_float_n(context, id, value, 4, min_value, max_value, layout);
        }

        static GUICore::ElementHandle slider_int_n(GUICore::IContext* context, GUICore::id_t id, i32* value,
            u8 count, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            Vector<f32> fractions(count);
            for(u32 i = 0; i < count; ++i)
            {
                fractions[i] = (value && max_value != min_value) ?
                    slider_fraction((f32)value[i], (f32)min_value, (f32)max_value) :
                    0.0f;
            }
            f32 interaction_fraction = 0.0f;
            u8 component_index = 0;
            if(value && max_value != min_value && slider_interaction_fraction(context, id, count, component_index, interaction_fraction))
            {
                f32 real_value = (f32)min_value + ((f32)max_value - (f32)min_value) * interaction_fraction;
                value[component_index] = clamp((i32)(real_value + (real_value >= 0.0f ? 0.5f : -0.5f)), min_value, max_value);
                fractions[component_index] = slider_fraction((f32)value[component_index], (f32)min_value, (f32)max_value);
            }
            return slider(context, id, fractions.cspan(), layout, count == 1 ? Name("slider_int") : Name("slider_int_n"));
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_int_n(context, id, value, 1, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int2(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_int_n(context, id, value, 2, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int3(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_int_n(context, id, value, 3, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle slider_int4(GUICore::IContext* context, GUICore::id_t id, i32* value,
            i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return slider_int_n(context, id, value, 4, min_value, max_value, layout);
        }

        static Ref<DragEditState> drag_edit_state(GUICore::IContext* context, GUICore::id_t id)
        {
            id_t state_id = GUICore::make_state_id<DragEditState>(id);
            Ref<DragEditState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<DragEditState>();
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static u8 drag_component_index(f32 y, f32 height, u8 count)
        {
            if(!count)
            {
                return 0;
            }
            f32 row_height = max(height / (f32)count, 1.0f);
            i32 row = (i32)(y / row_height);
            return (u8)clamp(row, 0, (i32)count - 1);
        }

        static GUICore::ElementHandle drag_draw(GUICore::IContext* context, GUICore::id_t id, Span<const String> labels,
            const GUICore::LayoutConfig& layout, const Name& debug_name)
        {
            GUICore::ElementHandle element = context->begin_element(id, debug_name);
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element);

            GUICore::InteractionState interaction = context->get_interaction_state(id);
            Float4U background = style_value(context, interaction.active ? Name("gui.editor.drag.background_active") :
                (interaction.hovered ? Name("gui.editor.drag.background_hovered") : Name("gui.editor.drag.background")),
                interaction.active ? GUICore::style_f32x4(Float4U(0.16f, 0.27f, 0.42f, 1.0f)) :
                (interaction.hovered ? GUICore::style_f32x4(Float4U(0.13f, 0.18f, 0.26f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.10f, 0.13f, 0.18f, 1.0f)))).number;
            Float4U border = style_value(context, Name("gui.editor.drag.border"),
                GUICore::style_f32x4(Float4U(0.18f, 0.25f, 0.34f, 1.0f))).number;
            Float4U text_color = style_value(context, Name("gui.editor.drag.text_color"),
                GUICore::style_f32x4(Float4U(0.95f, 0.96f, 0.98f, 1.0f))).number;
            f32 radius = style_value(context, Name("gui.editor.drag.radius"), GUICore::style_f32(4.0f)).number.x;
            f32 border_size = style_value(context, Name("gui.editor.drag.border_size"), GUICore::style_f32(1.0f)).number.x;
            f32 padding_x = style_value(context, Name("gui.editor.drag.padding_x"), GUICore::style_f32(8.0f)).number.x;
            f32 font_size = style_value(context, Name("gui.editor.drag.font_size"), GUICore::style_f32(14.0f)).number.x;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, 0.0f, 0.0f, 0.0f), border, radius);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(border_size, border_size, -border_size * 2.0f, -border_size * 2.0f), background, max(radius - border_size, 0.0f));
            u32 count = max((u32)labels.size(), 1u);
            for(u32 i = 0; i < count; ++i)
            {
                f32 row_begin = (f32)i / (f32)count;
                f32 row_size = 1.0f / (f32)count;
                const c8* text = i < labels.size() ? labels[i].c_str() : "";
                draw_scaled_text(context, RectF(padding_x, 0.0f, -padding_x * 2.0f, 0.0f),
                    Float4U(0.0f, row_begin, 1.0f, row_size), text, text_color, font_size);
            }
            context->end_element();
            return element;
        }

        static void drag_apply_float(GUICore::IContext* context, GUICore::id_t id, f32* value, u8 count,
            f32 speed, f32 min_value, f32 max_value)
        {
            Ref<DragEditState> state = drag_edit_state(context, id);
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            f32 interaction_height = max(interaction.clicked_element_rect.height, interaction.pointer_element_rect.height);
            Span<const GUICore::RoutedInputEvent> events = context->get_routed_input_events(id);
            for(const GUICore::RoutedInputEvent& routed : events)
            {
                const GUICore::InputEvent& event = routed.event;
                if(event.type == GUICore::InputEventType::pointer_down && event.button == GUICore::PointerButton::left)
                {
                    state->dragging = true;
                    state->component = drag_component_index(routed.element_position.y, interaction_height, count);
                    state->start_pointer_x = routed.has_element_position ? routed.element_position.x : event.position.x;
                    state->start_f32_values.resize(count);
                    for(u32 i = 0; i < count; ++i)
                    {
                        state->start_f32_values[i] = value ? value[i] : 0.0f;
                    }
                }
                else if(event.type == GUICore::InputEventType::pointer_move && state->dragging && value && state->start_f32_values.size() == count)
                {
                    f32 pointer_x = routed.has_element_position ? routed.element_position.x : event.position.x;
                    u8 component = min(state->component, (u8)(count - 1));
                    value[component] = clamp(state->start_f32_values[component] + (pointer_x - state->start_pointer_x) * speed,
                        min_value, max_value);
                }
                else if(event.type == GUICore::InputEventType::pointer_up && event.button == GUICore::PointerButton::left)
                {
                    state->dragging = false;
                }
            }
        }

        static GUICore::ElementHandle drag_float_n(GUICore::IContext* context, GUICore::id_t id, f32* value,
            u8 count, f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id && count);
            drag_apply_float(context, id, value, count, speed, min_value, max_value);
            Vector<String> labels(count);
            for(u32 i = 0; i < count; ++i)
            {
                strprintf(labels[i], "%.3f", value ? value[i] : 0.0f);
            }
            return drag_draw(context, id, labels.cspan(), layout, count == 1 ? Name("drag_float") : Name("drag_float_n"));
        }

        LUNA_GUI_API GUICore::ElementHandle drag_float(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_float_n(context, id, value, 1, speed, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle drag_float2(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_float_n(context, id, value, 2, speed, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle drag_float3(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_float_n(context, id, value, 3, speed, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle drag_float4(GUICore::IContext* context, GUICore::id_t id, f32* value,
            f32 speed, f32 min_value, f32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_float_n(context, id, value, 4, speed, min_value, max_value, layout);
        }

        static void drag_apply_int(GUICore::IContext* context, GUICore::id_t id, i32* value, u8 count,
            f32 speed, i32 min_value, i32 max_value)
        {
            Ref<DragEditState> state = drag_edit_state(context, id);
            GUICore::InteractionState interaction = context->get_interaction_state(id);
            f32 interaction_height = max(interaction.clicked_element_rect.height, interaction.pointer_element_rect.height);
            Span<const GUICore::RoutedInputEvent> events = context->get_routed_input_events(id);
            for(const GUICore::RoutedInputEvent& routed : events)
            {
                const GUICore::InputEvent& event = routed.event;
                if(event.type == GUICore::InputEventType::pointer_down && event.button == GUICore::PointerButton::left)
                {
                    state->dragging = true;
                    state->component = drag_component_index(routed.element_position.y, interaction_height, count);
                    state->start_pointer_x = routed.has_element_position ? routed.element_position.x : event.position.x;
                    state->start_i32_values.resize(count);
                    for(u32 i = 0; i < count; ++i)
                    {
                        state->start_i32_values[i] = value ? value[i] : 0;
                    }
                }
                else if(event.type == GUICore::InputEventType::pointer_move && state->dragging && value && state->start_i32_values.size() == count)
                {
                    f32 pointer_x = routed.has_element_position ? routed.element_position.x : event.position.x;
                    u8 component = min(state->component, (u8)(count - 1));
                    f32 real_value = (f32)state->start_i32_values[component] + (pointer_x - state->start_pointer_x) * speed;
                    value[component] = clamp((i32)(real_value + (real_value >= 0.0f ? 0.5f : -0.5f)), min_value, max_value);
                }
                else if(event.type == GUICore::InputEventType::pointer_up && event.button == GUICore::PointerButton::left)
                {
                    state->dragging = false;
                }
            }
        }

        static GUICore::ElementHandle drag_int_n(GUICore::IContext* context, GUICore::id_t id, i32* value,
            u8 count, f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            luassert(context && id && count);
            drag_apply_int(context, id, value, count, speed, min_value, max_value);
            Vector<String> labels(count);
            for(u32 i = 0; i < count; ++i)
            {
                strprintf(labels[i], "%d", value ? value[i] : 0);
            }
            return drag_draw(context, id, labels.cspan(), layout, count == 1 ? Name("drag_int") : Name("drag_int_n"));
        }

        LUNA_GUI_API GUICore::ElementHandle drag_int(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_int_n(context, id, value, 1, speed, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle drag_int2(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_int_n(context, id, value, 2, speed, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle drag_int3(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_int_n(context, id, value, 3, speed, min_value, max_value, layout);
        }

        LUNA_GUI_API GUICore::ElementHandle drag_int4(GUICore::IContext* context, GUICore::id_t id, i32* value,
            f32 speed, i32 min_value, i32 max_value, const GUICore::LayoutConfig& layout)
        {
            return drag_int_n(context, id, value, 4, speed, min_value, max_value, layout);
        }
    }
}
