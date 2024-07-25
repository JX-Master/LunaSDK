/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Button.cpp
* @author JXMaster
* @date 2024/7/25
*/
#include "../../Widgets/Button.hpp"
#include "../../WidgetDraw.hpp"

namespace Luna
{
    namespace GUI
    {
        enum class ButtonStateType
        {
            normal = 0,
            hovered = 1,
            pressed = 2,
        };
        struct ButtonState
        {
            lustruct("GUI::ButtonState", "9ebc5d25-7386-4229-a77e-2368321193b5");
            
            ButtonStateType state_type = ButtonStateType::normal;
        };

        LUNA_GUI_API RV Button::begin_update(IContext* ctx)
        {
            Ref<ButtonState> state = (ButtonState*)(ctx->get_widget_state(id));
            if(!state)
            {
                state = new_object<ButtonState>();
            }
            ctx->set_widget_state(id, state.get());
            button_state = state.get();
            return ok;
        }
        LUNA_GUI_API RV Button::draw(IContext* ctx, IDrawList* draw_list)
        {
            luassert(button_state);
            ButtonState* state = (ButtonState*)button_state.get();
            Float4U background_color, border_color;
            switch(state->state_type)
            {
                case ButtonStateType::normal:
                background_color = get_vattr(this, VATTR_BUTTON_BACKGROUND_COLOR, true, Float4U(0.3f, 0.3f, 0.3f, 1.0f));
                border_color = get_vattr(this, VATTR_BUTTON_BORDER_COLOR, true, Float4U(0.5f, 0.5f, 0.5f, 1.0f));
                break;
                case ButtonStateType::hovered:
                background_color = get_vattr(this, VATTR_BUTTON_HOVERED_BACKGROUND_COLOR, true, Float4U(0.7f, 0.7f, 0.7f, 1.0f));
                border_color = get_vattr(this, VATTR_BUTTON_HOVERED_BORDER_COLOR, true, Float4U(0.5f, 0.5f, 0.5f, 1.0f));
                break;
                case ButtonStateType::pressed:
                background_color = get_vattr(this, VATTR_BUTTON_PRESSED_BACKGROUND_COLOR, true, Float4U(0.5f, 0.5f, 0.5f, 1.0f));
                border_color = get_vattr(this, VATTR_BUTTON_PRESSED_BORDER_COLOR, true, Float4U(0.5f, 0.5f, 0.5f, 1.0f));
                break;
            }
            f32 border_width = get_sattr(this, SATTR_BUTTON_BORDER_WIDTH, true, 1.0f);
            f32 border_rounding = get_sattr(this, SATTR_BUTTON_ROUNDED_CORNER_RADIUS, this, 2.0f);
            // Draw background.
            if(background_color.w > 0)
            {
                if(border_rounding > 0)
                {
                    draw_rounded_rectangle_filled(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, background_color, border_rounding);
                }
                else
                {
                    draw_rectangle_filled(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, background_color);
                }
            }
            // Draw border.
            if(border_color.w > 0)
            {
                if(border_rounding > 0)
                {
                    draw_rounded_rectangle_bordered(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, border_color, border_rounding, border_width);
                }
                else
                {
                    draw_rectangle_bordered(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, border_color, border_width);
                }
            }
            // Draw content.
            lutry
            {
                if(body)
                {
                    luexp(body->draw(ctx, draw_list));    
                }
            }
            lucatchret;
            return ok;
        }
    }
}