/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorPrimitives.cpp
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

        LUNA_GUI_API GUICore::ElementHandle text(GUICore::IContext* context, GUICore::id_t id, const c8* text,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, text ? Name(text) : Name("text"));
            context->set_layout_config(element, layout);
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            command.color = style_value(context, Name("gui.editor.text.color"),
                GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f))).number;
            command.font_size = style_value(context, Name("gui.editor.text.font_size"), GUICore::style_f32(16.0f)).number.x;
            command.horizontal_alignment = VG::TextAlignment::begin;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, GUICore::id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout, ImageFlag flags)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, Name("image"));
            context->set_layout_config(element, layout);
            GUICore::DrawCommand command = element_rect_command(GUICore::DrawCommandType::image, Float4U(1.0f));
            command.texture = texture;
            command.min_texcoord = Float2U(0.0f, test_flags(flags, ImageFlag::flip_y) ? 1.0f : 0.0f);
            command.max_texcoord = Float2U(1.0f, test_flags(flags, ImageFlag::flip_y) ? 0.0f : 1.0f);
            command.nearest_sampler = test_flags(flags, ImageFlag::nearest);
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle shape(GUICore::IContext* context, GUICore::id_t id, const GUICore::ShapeDesc& shape,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, Name("shape"));
            context->set_layout_config(element, layout);
            GUICore::DrawCommand command = element_rect_command(GUICore::DrawCommandType::shape, Float4U(1.0f));
            command.shape = shape;
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle hit_box(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = context->begin_element(id, Name("hit_box"));
            context->set_layout_config(element, layout);
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            context->set_interactable(element, interactable);
            context->end_element();
            return element;
        }
    }
}
