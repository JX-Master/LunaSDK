/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Rectangle.cpp
* @author JXMaster
* @date 2024/5/8
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../../Context.hpp"
#include "../../Widgets.hpp"
#include "../../Widgets/Rectangle.hpp"
#include "../../WidgetDraw.hpp"

namespace Luna
{
    namespace GUI
    {
        LUNA_GUI_API RV Rectangle::draw(IContext* ctx, IDrawList* draw_list, IDrawList* overlay_draw_list)
        {
            Float4U background_color = get_vattr(this, VATTR_BACKGROUND_COLOR, true, Float4U(0));
            f32 rounding_radius = get_sattr(this, SATTR_ROUNDED_CORNER_RADIUS, true, 0.0f);
            if(background_color.w != 0)
            {
                if(rounding_radius > 0)
                {
                    draw_rounded_rectangle_filled(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, background_color, rounding_radius);
                }
                else
                {
                    draw_rectangle_filled(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, background_color);
                }
            }
            return ok;
        }
        LUNA_GUI_API Rectangle* rectangle(IWidgetBuilder* builder)
        {
            Ref<Rectangle> widget = new_object<Rectangle>();
            builder->add_widget(widget);
            return widget;
        }
    }
}