/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Rectangle.cpp
* @author JXMaster
* @date 2024/5/8
*/
#include "../../Context.hpp"
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/VG/Shapes.hpp>
#include "../../Widgets.hpp"
#include "Rectangle.hpp"

namespace Luna
{
    namespace GUI
    {
        RV Rectangle::render(IContext* ctx, VG::IShapeDrawList* draw_list)
        {
            lutry
            {
                Float4U background_color = get_vattr(VATTR_BACKGROUND_COLOR, true, Float4U(0));
                if(background_color.w != 0)
                {
                    auto& points = draw_list->get_shape_points();
                    u32 begin_command = (u32)points.size();
                    auto& io = ctx->get_io();
                    f32 screen_min_y = io.height - bounding_rect.bottom;
                    f32 screen_max_y = io.height - bounding_rect.top;
                    VG::ShapeBuilder::add_rectangle_filled(points, bounding_rect.left, screen_min_y, bounding_rect.right, screen_max_y);
                    u32 num_commands = (u32)points.size() - begin_command;
                    draw_list->draw_shape(begin_command, num_commands, 
                        {bounding_rect.left, screen_min_y}, {bounding_rect.right, screen_max_y}, 
                        {bounding_rect.left, screen_min_y}, {bounding_rect.right, screen_max_y},
                        Color::to_rgba8(background_color));
                }
                for(auto& c : children)
                {
                    luexp(c->render(ctx, draw_list));
                }
            }
            lucatchret;
            return ok;
        }
    }
}