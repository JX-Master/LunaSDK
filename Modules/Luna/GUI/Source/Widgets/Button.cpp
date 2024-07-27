/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Button.cpp
* @author JXMaster
* @date 2024/7/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Widgets/Button.hpp"
#include "../../WidgetDraw.hpp"
#include "../../Event.hpp"
#include "../../Widgets.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API f32 Button::get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y)
        {
            Float4U padding = get_vattr(this, VATTR_BUTTON_PADDING, true, Float4U(20, 20, 20, 20));
            f32 ret = 0;
            if(body)
            {
                ret = body->get_desired_size_x(type, suggested_size_y);
            }
            if(type == DesiredSizeType::preferred)
            {
                ret += padding.x + padding.z;
            }
            return ret;
        }
        constexpr Float4U BUTTON_DEFAULT_PADDING(20, 20, 20, 20);
        LUNA_GUI_API f32 Button::get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x)
        {
            Float4U padding = get_vattr(this, VATTR_BUTTON_PADDING, true, BUTTON_DEFAULT_PADDING);
            f32 ret = 0;
            if(body)
            {
                ret = body->get_desired_size_y(type, suggested_size_x);
            }
            if(type == DesiredSizeType::preferred)
            {
                ret += padding.y + padding.w;
            }
            return ret;
        }
        LUNA_GUI_API RV Button::begin_update(IContext* ctx)
        {
            Ref<ButtonState> state = (ButtonState*)(ctx->get_widget_state(id));
            if(!state)
            {
                state = new_object<ButtonState>();
            }
            ctx->set_widget_state(id, state.get());
            button_state = state.get();
            if(state->capture_mouse_event)
            {
                ctx->capture_event(this, typeof<MouseEvent>());
            }
            state->triggered = false;
            return ok;
        }
        LUNA_GUI_API RV Button::layout(IContext* ctx, const OffsetRectF& layout_rect)
        {
            lutry
            {
                luexp(Widget::layout(ctx, layout_rect));
                if(body)
                {
                    Float4 padding = get_vattr(this, VATTR_BUTTON_PADDING, true, BUTTON_DEFAULT_PADDING);
                    Float4 child_rect = Float4(layout_rect.left, layout_rect.top, layout_rect.right, layout_rect.bottom);
                    child_rect += padding * Float4(1, 1, -1, -1);
                    // Prevent negative rect.
                    child_rect.z = max(child_rect.z, child_rect.x);
                    child_rect.w = max(child_rect.w, child_rect.y);
                    luexp(body->layout(ctx, OffsetRectF(child_rect.x, child_rect.y, child_rect.z, child_rect.w)));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Button::handle_event(IContext* ctx, object_t e, bool& handled)
        {
            // Catch mouse event.
            MouseEvent* mouse_event = cast_object<MouseEvent>(e);
            if(mouse_event)
            {
                MouseMoveEvent* move_event = cast_object<MouseMoveEvent>(e);
                MouseButtonEvent* button_event = cast_object<MouseButtonEvent>(e);
                if(move_event)
                {
                    if(button_state->state_type == ButtonStateType::normal)
                    {
                        // Moved in.
                        button_state->state_type = ButtonStateType::hovered;
                        button_state->capture_mouse_event = true;
                    }
                    else if(button_state->state_type == ButtonStateType::hovered 
                        && !contains_point(move_event->x, move_event->y))
                    {
                        // Moved out.
                        button_state->state_type = ButtonStateType::normal;
                        button_state->capture_mouse_event = false;
                    }
                }
                else if(button_event && button_event->button == HID::MouseButton::left)
                {
                    if(button_event->pressed == true)
                    {
                        // Pressed.
                        button_state->state_type = ButtonStateType::pressed;
                        button_state->capture_mouse_event = true;
                        handled = true;
                    }
                    else
                    {
                        // Released.
                        if(contains_point(button_event->x, button_event->y))
                        {
                            button_state->state_type = ButtonStateType::hovered;
                            button_state->capture_mouse_event = true;
                            // Triggers button if mouse is released in the button region.
                            button_state->triggered = true;
                        }
                        else
                        {
                            button_state->state_type = ButtonStateType::normal;
                            button_state->capture_mouse_event = false;
                        }
                        handled = true;
                    }
                }
            }
            return ok;
        }
        LUNA_GUI_API RV Button::update(IContext* ctx)
        {
            lutry
            {
                luassert(button_state);
                if(button_state->triggered)
                {
                    luexp(on_click());
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API RV Button::draw(IContext* ctx, IDrawList* draw_list)
        {
            luassert(button_state);
            Float4U background_color, border_color;
            switch(button_state->state_type)
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
        LUNA_GUI_API void Button::add_child(IWidget* child)
        {
            body = child;
        }
        LUNA_GUI_API void Button::get_children(Vector<IWidget*>& out_children)
        {
            if(body)
            {
                out_children.push_back(body.get());
            }
        }
        LUNA_GUI_API usize Button::get_num_children()
        {
            return body ? 1 : 0;
        }
        LUNA_GUI_API Button* begin_button(IWidgetBuilder* builder, widget_id_t id, const Function<RV(void)>& on_click)
        {
            Ref<Button> widget = new_object<Button>();
            builder->push_id(id);
            widget->id = builder->get_id();
            widget->on_click = on_click;
            builder->add_widget(widget);
            builder->push_widget(widget);
            return widget;
        }
        LUNA_GUI_API void end_button(IWidgetBuilder* builder)
        {
            builder->pop_widget();
            builder->pop_id();
        }
        LUNA_GUI_API Button* button(IWidgetBuilder* builder, const Name& t, const Function<RV(void)>& on_click, widget_id_t id)
        {
            if(id == 0)
            {
                id = memhash32(t.c_str(), t.size());
            }
            Button* button = begin_button(builder, id, on_click);
            text(builder, t);
            end_button(builder);
            builder->set_current_widget(button);
            return button;
        }
    }
}