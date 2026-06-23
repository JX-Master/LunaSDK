/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorButtons.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/EditorWidgets.hpp>

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

        static GUICore::DrawCommand element_rect_command(GUICore::DrawCommandType type, const Float4U& color,
            f32 radius = 0.0f)
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            command.color = color;
            command.radius = radius;
            return command;
        }

        LUNA_GUI_API GUICore::ElementHandle begin_button(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const GUICore::LayoutInput& layout, bool enabled)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, label ? Name(label) : Name("button"));
            context->set_layout(element, layout);
            GUICore::Interactable interactable;
            set_flags(interactable.flags, GUICore::InteractableFlag::hit_test);
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            set_flags(interactable.flags, GUICore::InteractableFlag::disabled, !enabled);
            context->set_interactable(element, interactable);

            GUICore::InteractionState interaction = context->get_interaction_state(id);
            Name background_entry = !enabled ? Name("gui.editor.button.background_disabled") :
                (interaction.active ? Name("gui.editor.button.background_active") :
                (interaction.hovered ? Name("gui.editor.button.background_hovered") : Name("gui.editor.button.background")));
            GUICore::StyleValue background_default = !enabled ?
                GUICore::style_f32x4(Float4U(0.08f, 0.10f, 0.13f, 1.0f)) :
                (interaction.active ? GUICore::style_f32x4(Float4U(0.18f, 0.36f, 0.62f, 1.0f)) :
                (interaction.hovered ? GUICore::style_f32x4(Float4U(0.14f, 0.22f, 0.32f, 1.0f)) :
                GUICore::style_f32x4(Float4U(0.12f, 0.18f, 0.27f, 1.0f))));
            Float4U background = style_value(context, background_entry, background_default).number;
            f32 radius = style_value(context, Name("gui.editor.button.radius"), GUICore::style_f32(5.0f)).number.x;
            context->draw(element_rect_command(GUICore::DrawCommandType::rounded_rect, background, radius));
            return element;
        }

        LUNA_GUI_API void end_button(GUICore::IContext* context)
        {
            luassert(context);
            context->end_element();
        }

        LUNA_GUI_API GUICore::ElementHandle text_button(GUICore::IContext* context, GUICore::id_t id, const c8* text,
            const GUICore::LayoutInput& layout, bool enabled)
        {
            GUICore::ElementHandle element = begin_button(context, id, text, layout, enabled);
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(8.0f, 0.0f, -16.0f, 0.0f);
            command.color = style_value(context, enabled ? Name("gui.editor.button.text_color") :
                Name("gui.editor.button.text_disabled"), enabled ? GUICore::style_f32x4(Float4U(1.0f)) :
                GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
            command.font_size = style_value(context, Name("gui.editor.button.font_size"), GUICore::style_f32(16.0f)).number.x;
            command.horizontal_alignment = VG::TextAlignment::center;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
            end_button(context);
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle shape_button(GUICore::IContext* context, GUICore::id_t id, const c8* label,
            const GUICore::ShapeDesc& shape, const GUICore::LayoutInput& layout, f32 padding, bool enabled)
        {
            GUICore::ElementHandle element = begin_button(context, id, label, layout, enabled);
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::shape;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(padding, padding, -padding * 2.0f, -padding * 2.0f);
            command.color = style_value(context, enabled ? Name("gui.editor.shape_button.icon") :
                Name("gui.editor.shape_button.icon_disabled"), enabled ? GUICore::style_f32x4(Float4U(1.0f)) :
                GUICore::style_f32x4(Float4U(0.52f, 0.58f, 0.66f, 1.0f))).number;
            command.shape = shape;
            context->draw(command);
            end_button(context);
            return element;
        }
    }
}
