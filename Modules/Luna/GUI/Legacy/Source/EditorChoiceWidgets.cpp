/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorChoiceWidgets.cpp
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

        static void draw_relative_text(GUICore::IContext* context, const RectF& rect, const c8* text,
            const Float4U& color, f32 font_size, VG::TextAlignment horizontal_alignment = VG::TextAlignment::begin)
        {
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = rect;
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

        LUNA_GUI_API GUICore::ElementHandle selectable(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool selected,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "selectable");
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element, enabled);
            if(selected)
            {
                Float4U background = style_value(context, Name("gui.editor.selection.background"),
                    GUICore::style_f32x4(Float4U(0.16f, 0.25f, 0.38f, 1.0f))).number;
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                    RectF(0.0f, 0.0f, 0.0f, 0.0f), background, 4.0f);
            }
            Float4U color = style_value(context, enabled ? Name("gui.editor.selection.text_color") :
                Name("gui.editor.selection.text_disabled"), enabled ? GUICore::style_f32x4(Float4U(1.0f)) :
                GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.text.font_size"), GUICore::style_f32(15.0f)).number.x;
            draw_relative_text(context, RectF(8.0f, 0.0f, -16.0f, 0.0f), label, color, font_size);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool checked,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "checkbox");
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element, enabled);
            Float4U border = style_value(context, Name("gui.editor.check.border"),
                GUICore::style_f32x4(Float4U(0.55f, 0.64f, 0.76f, 1.0f))).number;
            Float4U fill = style_value(context, Name("gui.editor.check.fill"),
                GUICore::style_f32x4(Float4U(0.18f, 0.42f, 0.72f, 1.0f))).number;
            Float4U mark = style_value(context, Name("gui.editor.check.mark"), GUICore::style_f32x4(Float4U(1.0f))).number;
            Float4U disabled = style_value(context, Name("gui.editor.check.disabled"),
                GUICore::style_f32x4(Float4U(0.28f, 0.32f, 0.38f, 1.0f))).number;
            if(!enabled)
            {
                border = disabled;
                fill = disabled;
                mark = Float4U(0.62f, 0.66f, 0.72f, 1.0f);
            }
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, -8.0f, 16.0f, 16.0f),
                border, 3.0f, Float4U(0.0f, 0.5f, 0.0f, 0.0f));
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(2.0f, -6.0f, 12.0f, 12.0f),
                checked ? fill : Float4U(0.08f, 0.10f, 0.13f, 1.0f), 2.0f, Float4U(0.0f, 0.5f, 0.0f, 0.0f));
            if(checked)
            {
                draw_relative_line(context, Float2U(4.0f, -1.0f), Float2U(7.0f, 3.0f), mark, 2.0f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
                draw_relative_line(context, Float2U(7.0f, 3.0f), Float2U(13.0f, -4.0f), mark, 2.0f,
                    Float4U(0.0f, 0.5f, 0.0f, 0.5f));
            }
            Float4U text_color = style_value(context, enabled ? Name("gui.editor.text.color") :
                Name("gui.editor.text.disabled"), enabled ? GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.text.font_size"), GUICore::style_f32(16.0f)).number.x;
            draw_relative_text(context, RectF(24.0f, 0.0f, -28.0f, 0.0f), label, text_color, font_size);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle checkbox(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool* value,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            if(enabled && value && context->get_interaction_state(id).clicked)
            {
                *value = !*value;
            }
            return checkbox(context, id, label, value ? *value : false, layout, enabled);
        }

        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool selected,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "radio_button");
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element, enabled);
            Float4U border = style_value(context, Name("gui.editor.check.border"),
                GUICore::style_f32x4(Float4U(0.55f, 0.64f, 0.76f, 1.0f))).number;
            Float4U fill = style_value(context, Name("gui.editor.check.fill"),
                GUICore::style_f32x4(Float4U(0.18f, 0.42f, 0.72f, 1.0f))).number;
            if(!enabled)
            {
                border = style_value(context, Name("gui.editor.check.disabled"),
                    GUICore::style_f32x4(Float4U(0.28f, 0.32f, 0.38f, 1.0f))).number;
                fill = Float4U(0.42f, 0.46f, 0.52f, 1.0f);
            }
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(0.0f, -8.0f, 16.0f, 16.0f),
                border, 8.0f, Float4U(0.0f, 0.5f, 0.0f, 0.0f));
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(2.0f, -6.0f, 12.0f, 12.0f),
                Float4U(0.08f, 0.10f, 0.13f, 1.0f), 6.0f, Float4U(0.0f, 0.5f, 0.0f, 0.0f));
            if(selected)
            {
                draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect, RectF(5.0f, -3.0f, 6.0f, 6.0f),
                    fill, 3.0f, Float4U(0.0f, 0.5f, 0.0f, 0.0f));
            }
            Float4U text_color = style_value(context, enabled ? Name("gui.editor.text.color") :
                Name("gui.editor.text.disabled"), enabled ? GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.text.font_size"), GUICore::style_f32(16.0f)).number.x;
            draw_relative_text(context, RectF(24.0f, 0.0f, -28.0f, 0.0f), label, text_color, font_size);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool* value,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            if(enabled && value && context->get_interaction_state(id).clicked)
            {
                *value = true;
            }
            return radio_button(context, id, label, value ? *value : false, layout, enabled);
        }

        LUNA_GUI_API GUICore::ElementHandle radio_button(GUICore::IContext* context, GUICore::id_t id, const c8* label, i32* value,
            i32 button_value, const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            if(enabled && value && context->get_interaction_state(id).clicked)
            {
                *value = button_value;
            }
            return radio_button(context, id, label, value ? *value == button_value : false, layout, enabled);
        }

        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool checked,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "toggle_switch");
            context->set_layout_config(element, layout);
            set_basic_interactable(context, element, enabled);
            f32 target = checked ? 1.0f : 0.0f;
            f32 animation = target;
            id_t state_id = GUICore::make_state_id<SwitchAnimationState>(id);
            if(object_t state_obj = context->get_state(state_id))
            {
                if(SwitchAnimationState* state = cast_object<SwitchAnimationState>(state_obj); state && state->initialized)
                {
                    animation = state->animation;
                }
            }
            f32 speed = style_value(context, Name("gui.editor.switch.animation_speed"), GUICore::style_f32(14.0f)).number.x;
            f32 blend = clamp(context->get_frame_desc().delta_time * speed, 0.0f, 1.0f);
            animation += (target - animation) * blend;
            animation = clamp(animation, 0.0f, 1.0f);
            Ref<SwitchAnimationState> next_state = new_object<SwitchAnimationState>();
            next_state->animation = animation;
            next_state->initialized = true;
            lupanic_if_failed(context->set_state(state_id, next_state.object(), GUICore::StateLifetime::next_frame));

            Float4U track_size_value = style_value(context, Name("gui.editor.switch.track_size"),
                GUICore::style_f32x2(Float2U(44.0f, 22.0f))).number;
            Float2U track_size(track_size_value.x, track_size_value.y);
            f32 knob_size = style_value(context, Name("gui.editor.switch.knob_size"), GUICore::style_f32(18.0f)).number.x;
            f32 knob_margin = style_value(context, Name("gui.editor.switch.knob_margin"), GUICore::style_f32(2.0f)).number.x;
            Float4U off_track = style_value(context, Name("gui.editor.switch.off_track"),
                GUICore::style_f32x4(Float4U(0.12f, 0.14f, 0.16f, 1.0f))).number;
            Float4U on_track = style_value(context, Name("gui.editor.switch.on_track"),
                GUICore::style_f32x4(Float4U(0.20f, 0.55f, 0.32f, 1.0f))).number;
            Float4U off_knob = style_value(context, Name("gui.editor.switch.off_knob"),
                GUICore::style_f32x4(Float4U(0.78f, 0.80f, 0.84f, 1.0f))).number;
            Float4U on_knob = style_value(context, Name("gui.editor.switch.on_knob"),
                GUICore::style_f32x4(Float4U(1.0f))).number;
            if(!enabled)
            {
                off_track = style_value(context, Name("gui.editor.switch.disabled_track"),
                    GUICore::style_f32x4(Float4U(0.10f, 0.11f, 0.13f, 1.0f))).number;
                on_track = off_track;
                off_knob = style_value(context, Name("gui.editor.switch.disabled_knob"),
                    GUICore::style_f32x4(Float4U(0.42f, 0.46f, 0.52f, 1.0f))).number;
                on_knob = off_knob;
            }
            f32 track_y = max((28.0f - track_size.y) * 0.5f, 0.0f);
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(2.0f, track_y, track_size.x, track_size.y), smooth_color(off_track, on_track, animation), track_size.y * 0.5f);
            f32 knob_x = 2.0f + knob_margin + (track_size.x - knob_size - knob_margin * 2.0f) * animation;
            draw_relative_rect(context, GUICore::DrawCommandType::rounded_rect,
                RectF(knob_x, track_y + knob_margin, knob_size, knob_size), smooth_color(off_knob, on_knob, animation), knob_size * 0.5f);
            Float4U text_color = style_value(context, enabled ? Name("gui.editor.text.color") :
                Name("gui.editor.text.disabled"), enabled ? GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
            f32 font_size = style_value(context, Name("gui.editor.switch.font_size"), GUICore::style_f32(16.0f)).number.x;
            f32 label_offset = style_value(context, Name("gui.editor.switch.label_offset"), GUICore::style_f32(56.0f)).number.x;
            draw_relative_text(context, RectF(label_offset, 0.0f, -label_offset, 0.0f), label, text_color, font_size);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle toggle_switch(GUICore::IContext* context, GUICore::id_t id, const c8* label, bool* value,
            const GUICore::LayoutConfig& layout, bool enabled)
        {
            luassert(context && id);
            if(enabled && value && context->get_interaction_state(id).clicked)
            {
                *value = !*value;
            }
            return toggle_switch(context, id, label, value ? *value : false, layout, enabled);
        }
    }
}
