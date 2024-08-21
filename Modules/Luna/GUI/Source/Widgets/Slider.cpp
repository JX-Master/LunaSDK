/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Slider.cpp
* @author JXMaster
* @date 2024/7/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Widgets/Slider.hpp"
#include "../../Context.hpp"
#include "../../Event.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API f32 Slider::get_desired_size_x(DesiredSizeType type, const f32* suggested_size_y)
        {
            bool attr_found = false;
            f32 ret = get_desired_size_x_attr(this, type, &attr_found);
            if(attr_found) return ret;
            switch(type)
            {
                case DesiredSizeType::required: return 0;
                case DesiredSizeType::preferred: return 100;
                case DesiredSizeType::filling: return 0;
            }
            return 0;
        }
        LUNA_GUI_API f32 Slider::get_desired_size_y(DesiredSizeType type, const f32* suggested_size_x)
        {
            bool attr_found = false;
            f32 ret = get_desired_size_y_attr(this, type, &attr_found);
            if(attr_found) return ret;
            switch(type)
            {
                case DesiredSizeType::required: return 0;
                case DesiredSizeType::preferred: return 10;
                case DesiredSizeType::filling: return 0;
            }
            return 0;
        }
        LUNA_GUI_API RV Slider::begin_update(IContext* ctx)
        {
            Ref<SliderState> state = (SliderState*)(ctx->get_widget_state(id));
            if(!state)
            {
                state = new_object<SliderState>();
            }
            ctx->set_widget_state(id, state);
            slider_state = state;
            if(state->sliding)
            {
                ctx->capture_event(this, typeof<MouseEvent>());
            }
            return ok;
        }
        LUNA_GUI_API RV Slider::handle_event(IContext* ctx, object_t e, bool& handled)
        {
            MouseEvent* mouse_event = cast_object<MouseEvent>(e);
            if(mouse_event)
            {
                MouseMoveEvent* move_event = cast_object<MouseMoveEvent>(e);
                MouseButtonEvent* button_event = cast_object<MouseButtonEvent>(e);
                if(move_event)
                {
                    
                }
            }
            return ok;
        }
        LUNA_GUI_API RV Slider::update(IContext* ctx)
        {
            return ok;
        }
        LUNA_GUI_API RV Slider::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            return ok;
        }
    }
}