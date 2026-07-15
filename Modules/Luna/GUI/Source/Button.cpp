/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Button.cpp
* @author JXMaster
* @date 2026/7/13
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
            struct ButtonData
            {
                bool enabled = true;
                ButtonVisualState* state = nullptr;
            };

            static Float4U mix_color(const Float4U& a, const Float4U& b, f32 factor)
            {
                return a + (b - a) * clamp(factor, 0.0f, 1.0f);
            }

            static RV draw_button(GUICore::IContext* context, const GUICore::ElementHandle& element,
                GUICore::DrawPhase, void* userdata)
            {
                ButtonData* data = (ButtonData*)userdata;
                if(!data) return ok;
                Float4U color = style_color(context, element, data->enabled ? "gui.button.background" :
                    "gui.button.background_disabled", data->enabled ? Float4U(0.12f, 0.18f, 0.27f, 1.0f) :
                    Float4U(0.09f, 0.11f, 0.14f, 1.0f));
                if(data->enabled && data->state)
                {
                    color = mix_color(color, style_color(context, element, "gui.button.background_hovered",
                        Float4U(0.16f, 0.25f, 0.38f, 1.0f)), data->state->hovered);
                    color = mix_color(color, style_color(context, element, "gui.button.background_active",
                        Float4U(0.20f, 0.36f, 0.58f, 1.0f)), data->state->active);
                }
                GUICore::DrawCommand command;
                command.type = GUICore::DrawCommandType::rounded_rect;
                command.rect_reference = GUICore::DrawCommandRectReference::element;
                command.color = color;
                command.radius = style_scalar(context, element, "gui.button.radius", 4.0f);
                context->draw(command);
                return ok;
            }

            static GUICore::FlexLayoutDesc* configure_button_layout(GUICore::IContext* context,
                const GUICore::ElementHandle& element)
            {
                GUICore::FlexLayoutDesc* flex = allocate_frame<GUICore::FlexLayoutDesc>(context);
                flex->axis = GUICore::LayoutAxis::x;
                flex->main_alignment = GUICore::FlexAlignment::center;
                flex->cross_alignment = GUICore::FlexAlignment::center;
                GUICore::LayoutCallbackConfig callbacks;
                callbacks.algorithm = Name("gui.button");
                callbacks.measure_callback = GUICore::measure_flex;
                callbacks.callback = GUICore::layout_flex;
                callbacks.userdata = flex;
                context->set_layout_callback_config(element, callbacks);
                return flex;
            }
        }

        LUNA_GUI_API GUICore::ElementHandle begin_button(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::LayoutConfig& layout, const ButtonDesc& desc)
        {
            luassert(context && id);
            GUICore::LayoutConfig resolved_layout = layout;
            Float4U padding_value = context->get_style_value(context->current_style(), Name("gui.button.padding"),
                GUICore::style_f32x2(Float2U(10.0f, 6.0f))).number;
            Float2U padding(padding_value.x, padding_value.y);
            if(resolved_layout.padding.x == 0.0f && resolved_layout.padding.y == 0.0f &&
                resolved_layout.padding.z == 0.0f && resolved_layout.padding.w == 0.0f)
            {
                resolved_layout.padding = Float4U(padding.x, padding.y, padding.x, padding.y);
            }
            GUICore::ElementHandle element = Internal::begin_element(context, id, label ? label : "Button",
                resolved_layout);
            Internal::set_interactable(context, element, desc.enabled);
            Internal::configure_button_layout(context, element);
            Ref<Internal::ButtonVisualState> state = Internal::widget_state<Internal::ButtonVisualState>(context, id);
            Internal::ButtonData* data = Internal::allocate_frame<Internal::ButtonData>(context);
            data->enabled = desc.enabled;
            data->state = state.get();
            GUICore::DrawConfig draw;
            draw.name = Name("gui.button");
            draw.callback = Internal::draw_button;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            Internal::ButtonAction* action = Internal::allocate_frame<Internal::ButtonAction>(context);
            action->id = id;
            action->enabled = desc.enabled;
            action->state = state.get();
            Internal::add_action(context, Internal::ActionType::button, id, action);
            return element;
        }

        LUNA_GUI_API void end_button(GUICore::IContext* context)
        {
            luassert(context);
            context->end_element();
        }

        LUNA_GUI_API GUICore::ElementHandle text_button(GUICore::IContext* context, id_t id, const c8* value,
            const GUICore::LayoutConfig& layout, const ButtonDesc& desc)
        {
            GUICore::ElementHandle element = begin_button(context, id, value, layout, desc);
            TextDesc text_desc;
            text_desc.horizontal_alignment = TextAlignment::center;
            text_desc.vertical_alignment = TextAlignment::center;
            text_desc.color = desc.enabled ? Internal::style_color(context, element, "gui.button.text", Float4U(1.0f)) :
                Internal::style_color(context, element, "gui.text.disabled", Float4U(0.48f, 0.52f, 0.58f, 1.0f));
            GUICore::LayoutConfig text_layout;
            text_layout.width.kind = GUICore::SizeKind::fit;
            text_layout.height.kind = GUICore::SizeKind::fit;
            text(context, GUICore::make_scoped_id(id, "text"), value, text_layout, text_desc);
            end_button(context);
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle shape_button(GUICore::IContext* context, id_t id, const c8* label,
            const GUICore::ShapeDesc& value, const GUICore::LayoutConfig& layout, const ShapeButtonDesc& desc)
        {
            ButtonDesc button_desc;
            button_desc.enabled = desc.enabled;
            GUICore::ElementHandle element = begin_button(context, id, label, layout, button_desc);
            GUICore::LayoutConfig shape_layout;
            shape_layout.width.kind = GUICore::SizeKind::percent;
            shape_layout.width.value = 1.0f;
            shape_layout.height.kind = GUICore::SizeKind::percent;
            shape_layout.height.value = 1.0f;
            shape_layout.margin = Float4U(desc.padding);
            ShapeWidgetDesc shape_desc;
            shape_desc.tint = desc.tint.w >= 0.0f ? desc.tint :
                Internal::style_color(context, element, desc.enabled ? "gui.button.text" : "gui.text.disabled",
                    desc.enabled ? Float4U(1.0f) : Float4U(0.48f, 0.52f, 0.58f, 1.0f));
            shape(context, GUICore::make_scoped_id(id, "shape"), value, shape_layout, shape_desc);
            end_button(context);
            return element;
        }
    }
}
