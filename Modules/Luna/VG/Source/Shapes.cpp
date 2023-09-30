/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Shape.cpp
* @author JXMaster
* @date 2023/9/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "../Shapes.hpp"
#include <Luna/Runtime/Math/Vector.hpp>

namespace Luna
{
    namespace VG
    {
        namespace ShapeBuilder
        {
            LUNA_VG_API void circle_to(Vector<f32>& points, f32 radius, f32 begin, f32 end)
			{
				// round to positive.
				if (begin < 0 || end < 0)
				{
					f32 m = min(begin, end);
					i32 round = i32(-m / 360.0f) + 1;
					m = 360.0f * (f32)round;
					begin += m;
					end += m;
				}
				f32 cur = begin;
				if (end > begin)
				{
					while (cur < end)
					{
						i32 quad_count = i32(cur / 90.0f);
						i32 quad = quad_count % 4;
						f32 next = min(end, (quad_count + 1) * 90.0f);
						points.insert(points.end(), { COMMAND_CIRCLE_Q1 + (f32)quad, radius, cur, next });
						cur = next;
					}
				}
				else if(end < begin)
				{
					while (cur > end)
					{
						i32 quad_count = i32(cur / 90.0f);
						if (quad_count * 90.0f == cur) --quad_count;
						i32 quad = quad_count % 4;
						f32 next = max(end, quad_count * 90.0f);
						points.insert(points.end(), { COMMAND_CIRCLE_Q1 + (f32)quad, radius, cur, next });
						cur = next;
					}
				}
			}
			LUNA_VG_API void add_rectangle_filled(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y)
			{
				move_to(points, min_x, min_y);
				line_to(points, min_x, max_y);
				line_to(points, max_x, max_y);
				line_to(points, max_x, min_y);
				line_to(points, min_x, min_y);
			}
			LUNA_VG_API void add_rectangle_bordered(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 border_width, f32 border_offset)
			{
				f32 border_width_div_2 = border_width / 2.0f;
				f32 border_offset_outer = border_width_div_2 + border_offset;
				f32 border_offset_inner = border_width_div_2 - border_offset;
				f32 outer_min_x = min_x - border_offset_outer;
				f32 outer_min_y = min_y - border_offset_outer;
				f32 outer_max_x = max_x + border_offset_outer;
				f32 outer_max_y = max_y + border_offset_outer;
				f32 inner_min_x = min_x + border_offset_inner;
				f32 inner_min_y = min_y + border_offset_inner;
				f32 inner_max_x = max_x - border_offset_inner;
				f32 inner_max_y = max_y - border_offset_inner;
				move_to(points, outer_min_x, outer_min_y);
				line_to(points, outer_min_x, outer_max_y);
				line_to(points, outer_max_x, outer_max_y);
				line_to(points, outer_max_x, outer_min_y);
				line_to(points, outer_min_x, outer_min_y);
				move_to(points, inner_min_x, inner_min_y);
				line_to(points, inner_max_x, inner_min_y);
				line_to(points, inner_max_x, inner_max_y);
				line_to(points, inner_min_x, inner_max_y);
				line_to(points, inner_min_x, inner_min_y);
			}
			LUNA_VG_API void add_line(Vector<f32>& points, f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 border_width, f32 border_offset)
			{
				Float2 p1(p1_x, p1_y);
				Float2 p2(p2_x, p2_y);
				Float2 n(p1_y - p2_y, p2_x - p1_x);
				n = normalize(n);
				f32 border_width_div_2 = border_width / 2.0f;
				Float2 n1 = n * (border_width_div_2 + border_offset);
				Float2 n2 = -n * (border_width_div_2 - border_offset);
				Float2 p1_1 = p1 + n1;
				Float2 p1_2 = p1 + n2;
				Float2 p2_1 = p2 + n1;
				Float2 p2_2 = p2 + n2;
				move_to(points, p1_2.x, p1_2.y);
				line_to(points, p1_1.x, p1_1.y);
				line_to(points, p2_1.x, p2_1.y);
				line_to(points, p2_2.x, p2_2.y);
				line_to(points, p1_2.x, p1_2.y);
			}
			LUNA_VG_API void add_rounded_rectangle_filled(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius)
			{
				move_to(points, min_x, min_y + radius);
				line_to(points, min_x, max_y - radius);
				circle_to(points, radius, 180.0f, 90.0f);
				line_to(points, max_x - radius, max_y);
				circle_to(points, radius, 90.0f, 0.0f);
				line_to(points, max_x, min_y + radius);
				circle_to(points, radius, 0.0f, -90.0f);
				line_to(points, min_x + radius, min_y);
				circle_to(points, radius, -90.0f, -180.0f);
			}
			LUNA_VG_API void add_rounded_rectangle_bordered(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius, f32 border_width, f32 border_offset)
			{
				f32 border_width_div_2 = border_width / 2.0f;
				f32 border_offset_outer = border_width_div_2 + border_offset;
				f32 border_offset_inner = border_width_div_2 - border_offset;
				f32 outer_min_x = min_x - border_offset_outer;
				f32 outer_min_y = min_y - border_offset_outer;
				f32 outer_max_x = max_x + border_offset_outer;
				f32 outer_max_y = max_y + border_offset_outer;
				f32 inner_min_x = min_x + border_offset_inner;
				f32 inner_min_y = min_y + border_offset_inner;
				f32 inner_max_x = max_x - border_offset_inner;
				f32 inner_max_y = max_y - border_offset_inner;
				f32 inner_radius = max(radius - border_offset_inner, 0.0f);
				f32 outer_radius = max(radius + border_offset_outer, 0.0f);
				if(outer_radius > 0.0f)
				{
					move_to(points, outer_min_x, outer_min_y + outer_radius);
					line_to(points, outer_min_x, outer_max_y - outer_radius);
					circle_to(points, outer_radius, 180.0f, 90.0f);
					line_to(points, outer_max_x - outer_radius, outer_max_y);
					circle_to(points, outer_radius, 90.0f, 0.0f);
					line_to(points, outer_max_x, outer_min_y + outer_radius);
					circle_to(points, outer_radius, 0.0f, -90.0f);
					line_to(points, outer_min_x + outer_radius, outer_min_y);
					circle_to(points, outer_radius, -90.0f, -180.0f);
				}
				else
				{
					move_to(points, outer_min_x, outer_min_y);
					line_to(points, outer_min_x, outer_max_y);
					line_to(points, outer_max_x, outer_max_y);
					line_to(points, outer_max_x, outer_min_y);
					line_to(points, outer_min_x, outer_min_y);
				}
				if(inner_radius > 0.0f)
				{
					move_to(points, inner_min_x + inner_radius, inner_min_y);
					line_to(points, inner_max_x - inner_radius, inner_min_y);
					circle_to(points, inner_radius, -90.0f, 0.0f);
					line_to(points, inner_max_x, inner_max_y - inner_radius);
					circle_to(points, inner_radius, 0.0f, 90.0f);
					line_to(points, inner_min_x + inner_radius, inner_max_y);
					circle_to(points, inner_radius, 90.0f, 180.0f);
					line_to(points, inner_min_x, inner_min_y + inner_radius);
					circle_to(points, inner_radius, 180.0f, 270.0f);
				}
				else
				{
					move_to(points, inner_min_x, inner_min_y);
					line_to(points, inner_max_x, inner_min_y);
					line_to(points, inner_max_x, inner_max_y);
					line_to(points, inner_min_x, inner_max_y);
					line_to(points, inner_min_x, inner_min_y);
				}
			}
			LUNA_VG_API void add_circle_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius)
			{
				move_to(points, center_x, center_y + radius);
				circle_to(points, radius, 90.0f, -270.0f);
			}
			LUNA_VG_API void add_circle_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 border_width, f32 border_offset)
			{
				f32 border_width_div_2 = border_width / 2.0f;
				f32 border_offset_outer = border_width_div_2 + border_offset;
				f32 border_offset_inner = border_width_div_2 - border_offset;
				f32 inner_radius = max(radius - border_offset_inner, 0.0f);
				f32 outer_radius = max(radius + border_offset_outer, 0.0f);
				if(outer_radius > 0.0f)
				{
					move_to(points, center_x, center_y + outer_radius);
					circle_to(points, outer_radius, 90.0f, -270.0f);
				}
				if(inner_radius > 0.0f)
				{
					move_to(points, center_x, center_y + inner_radius);
					circle_to(points, inner_radius, -270.0f, 90.0f);
				}
			}
        }
    }
}