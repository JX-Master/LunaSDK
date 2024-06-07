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
#include "Rectangle.hpp"
#include "../../WidgetDraw.hpp"

namespace Luna
{
    namespace GUI
    {
        RV Rectangle::draw(IContext* ctx, VG::IShapeDrawList* draw_list)
        {
            lutry
            {
                Float4U background_color = get_vattr(VATTR_BACKGROUND_COLOR, true, Float4U(0));
                if(background_color.w != 0)
                {
                    draw_rectangle_filled(ctx, draw_list, bounding_rect.left, bounding_rect.top, bounding_rect.right, bounding_rect.bottom, background_color);
                }
                for(auto& c : children)
                {
                    luexp(c->draw(ctx, draw_list));
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_GUI_API void begin_rectangle(IContext* ctx)
        {
            Ref<Rectangle> widget = new_object<Rectangle>();
            ctx->add_widget(widget);
            ctx->push_widget(widget);
        }
        LUNA_GUI_API void end_rectangle(IContext* ctx)
        {
            ctx->pop_widget();
        }
    }
}