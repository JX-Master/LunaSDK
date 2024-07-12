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
            LUNA_VG_API void axis_aligned_ellipse_to(Vector<f32>& points, f32 radius_x, f32 radius_y, f32 begin, f32 end)
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
                        points.insert(points.end(), { COMMAND_AXIS_ALIGNED_ELLIPSE_Q1 + (f32)quad, radius_x, radius_y, cur, next });
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
                        points.insert(points.end(), { COMMAND_AXIS_ALIGNED_ELLIPSE_Q1 + (f32)quad, radius_x, radius_y, cur, next });
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
            LUNA_VG_API void add_line(Vector<f32>& points, f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 width, f32 offset)
            {
                Float2 p1(p1_x, p1_y);
                Float2 p2(p2_x, p2_y);
                Float2 n(p1_y - p2_y, p2_x - p1_x);
                n = normalize(n);
                f32 width_div_2 = width / 2.0f;
                Float2 n1 = n * (width_div_2 + offset);
                Float2 n2 = -n * (width_div_2 - offset);
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
            LUNA_VG_API void add_arc_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 begin_angle, f32 end_angle)
            {
                move_to(points, center_x, center_y);
                f32 begin_angle_rad = deg_to_rad(begin_angle);
                f32 x1 = center_x + radius * cosf(begin_angle_rad);
                f32 y1 = center_y + radius * sinf(begin_angle_rad);
                line_to(points, x1, y1);
                circle_to(points, radius, begin_angle, end_angle);
                line_to(points, center_x, center_y);
            }
            LUNA_VG_API void add_arc_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 begin_angle, f32 end_angle, f32 border_width, f32 border_offset)
            {
                f32 border_width_div_2 = border_width / 2.0f;
                f32 border_offset_outer = border_width_div_2 + border_offset;
                f32 border_offset_inner = border_width_div_2 - border_offset;
                f32 inner_radius = max(radius - border_offset_inner, 0.0f);
                f32 outer_radius = max(radius + border_offset_outer, 0.0f);
                if(outer_radius == 0.0f) return;
                if(inner_radius == 0.0f)
                {
                    add_arc_filled(points, center_x, center_y, outer_radius, begin_angle, end_angle);
                }
                else
                {
                    f32 begin_angle_rad = deg_to_rad(begin_angle);
                    f32 end_angle_rad = deg_to_rad(end_angle);
                    f32 inner_x1 = center_x + inner_radius * cosf(begin_angle_rad);
                    f32 inner_y1 = center_y + inner_radius * sinf(begin_angle_rad);
                    f32 outer_x1 = center_x + outer_radius * cosf(begin_angle_rad);
                    f32 outer_y1 = center_y + outer_radius * sinf(begin_angle_rad);
                    f32 inner_x2 = center_x + inner_radius * cosf(end_angle_rad);
                    f32 inner_y2 = center_y + inner_radius * sinf(end_angle_rad);
                    move_to(points, inner_x1, inner_y1);
                    line_to(points, outer_x1, outer_y1);
                    circle_to(points, outer_radius, begin_angle, end_angle);
                    line_to(points, inner_x2, inner_y2);
                    circle_to(points, inner_radius, end_angle, begin_angle);
                }
            }
            LUNA_VG_API void add_axis_aligned_ellipse_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius_x, f32 radius_y)
            {
                move_to(points, center_x, center_y + radius_y);
                axis_aligned_ellipse_to(points, radius_x, radius_y, 90.0f, -270.0f);
            }
            LUNA_VG_API void add_axis_aligned_ellipse_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius_x, f32 radius_y, f32 border_width, f32 border_offset)
            {
                f32 border_width_div_2 = border_width / 2.0f;
                f32 border_offset_outer = border_width_div_2 + border_offset;
                f32 border_offset_inner = border_width_div_2 - border_offset;
                f32 inner_radius_x = max(radius_x - border_offset_inner, 0.0f);
                f32 outer_radius_x = max(radius_x + border_offset_outer, 0.0f);
                f32 inner_radius_y = max(radius_y - border_offset_inner, 0.0f);
                f32 outer_radius_y = max(radius_y + border_offset_outer, 0.0f);
                if(outer_radius_x > 0.0f && outer_radius_y > 0.0f)
                {
                    move_to(points, center_x, center_y + outer_radius_y);
                    axis_aligned_ellipse_to(points, outer_radius_x, outer_radius_y, 90.0f, -270.0f);
                }
                if(inner_radius_x > 0.0f && inner_radius_y > 0.0f)
                {
                    move_to(points, center_x, center_y + inner_radius_y);
                    axis_aligned_ellipse_to(points, inner_radius_x, inner_radius_y, -270.0f, 90.0f);
                }
            }
            LUNA_VG_API void add_triangle_filled(Vector<f32>& points, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
            {
                move_to(points, x1, y1);
                line_to(points, x2, y2);
                line_to(points, x3, y3);
                line_to(points, x1, y1);
            }
            inline Float2 get_triangle_border_point_offset(f32 x, f32 y, f32 x1, f32 y1, f32 x2, f32 y2)
            {
                Float2 dir1(x1 - x, y1 - y);
                Float2 dir2(x2 - x, y2 - y);
                dir1 = normalize(dir1);
                dir2 = normalize(dir2);
                f32 theta = acosf(dot(dir1, dir2)) / 2;
                Float2 dir = normalize(dir1 + dir2);
                if(theta < F32_EPSILON) return Float2(0, 0);
                f32 d = 1.0f / sinf(theta);
                return dir * d;
            }
            LUNA_VG_API void add_triangle_bordered(Vector<f32>& points, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 border_width, f32 border_offset)
            {
                Float2 dir1(x2 - x1, y2 - y1);
                Float2 dir2(x3 - x1, y3 - y1);
                Float2 dir3(x3 - x2, y3 - y2);
                f32 s = abs(cross(dir1, dir2).x);
                f32 max_b = s / (length(dir1) + length(dir2) + length(dir3));
                Float2 offset1 = get_triangle_border_point_offset(x1, y1, x2, y2, x3, y3);
                Float2 offset2 = get_triangle_border_point_offset(x2, y2, x1, y1, x3, y3);
                Float2 offset3 = get_triangle_border_point_offset(x3, y3, x1, y1, x2, y2);
                if(offset1 == Float2(0, 0) || offset2 == Float2(0, 0) || offset3 == Float2(0, 0))
                {
                    add_line(points, x1, y1, x2, y2, border_width, border_offset);
                    add_line(points, x1, y1, x3, y3, border_width, border_offset);
                    add_line(points, x2, y2, x3, y3, border_width, border_offset);
                    return;
                }
                f32 half_border_width = border_width / 2.0f;
                f32 out_offset = half_border_width + border_offset;
                if (out_offset <= -max_b) return;
                // draw outline.
                Float2 p1_out = Float2(x1, y1) - offset1 * out_offset;
                Float2 p2_out = Float2(x2, y2) - offset2 * out_offset;
                Float2 p3_out = Float2(x3, y3) - offset3 * out_offset;
                move_to(points, p1_out.x, p1_out.y);
                line_to(points, p2_out.x, p2_out.y);
                line_to(points, p3_out.x, p3_out.y);
                line_to(points, p1_out.x, p1_out.y);
                // draw inline.
                f32 in_offset = half_border_width - border_offset;
                if(in_offset >= max_b) return;
                Float2 p1_in = Float2(x1, y1) + offset1 * in_offset;
                Float2 p2_in = Float2(x2, y2) + offset2 * in_offset;
                Float2 p3_in = Float2(x3, y3) + offset3 * in_offset;
                move_to(points, p1_in.x, p1_in.y);
                line_to(points, p3_in.x, p3_in.y);
                line_to(points, p2_in.x, p2_in.y);
                line_to(points, p1_in.x, p1_in.y);
            }
            LUNA_VG_API void add_polygon_filled(Vector<f32>& points, Span<const Float2U> vertices)
            {
                if(vertices.size() < 3) return;
                move_to(points, vertices[0].x, vertices[0].y);
                for(usize i = 1; i < vertices.size(); ++i)
                {
                    line_to(points, vertices[i].x, vertices[i].y);
                }
                line_to(points, vertices[0].x, vertices[0].y);
            }
            inline Float2 calc_polyline_offset(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 offset)
            {
                Float2 dir1 = normalize(Float2(x2 - x1, y2 - y1));
                Float2 dir2 = normalize(Float2(x3 - x2, y3 - y2));
                dir1 = Float2(-dir1.y, dir1.x);
                dir2 = Float2(-dir2.y, dir2.x);
                Float2 dir = dir1 + dir2;
                if(length(dir) <= F32_EPSILON)
                {
                    return dir1 * offset;
                }
                return normalize(dir) * offset;
            }
            LUNA_VG_API void add_polygon_bordered(Vector<f32>& points, Span<const Float2U> vertices, f32 border_width, f32 border_offset)
            {
                if(vertices.size() < 3) return;
                // draw outside polygon.
                f32 half_border_width = border_width / 2.0f;
                f32 outer_offset = half_border_width + border_offset;
                Float2 d = calc_polyline_offset(vertices.back().x, vertices.back().y, vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y, outer_offset);
                Float2 first_point(vertices[0].x + d.x, vertices[0].y + d.y);
                move_to(points, first_point.x, first_point.y);
                for(usize i = 1; i < vertices.size(); ++i)
                {
                    usize last_point = i == vertices.size() - 1 ? 0 : i + 1;
                    d = calc_polyline_offset(vertices[i - 1].x, vertices[i - 1].y, vertices[i].x, vertices[i].y, vertices[last_point].x, vertices[last_point].y, outer_offset);
                    line_to(points, vertices[i].x + d.x, vertices[i].y + d.y);
                }
                line_to(points, first_point.x, first_point.y);
                // draw inside polygon.
                f32 inner_offset = half_border_width - border_offset;
                d = calc_polyline_offset(vertices[1].x, vertices[1].y, vertices[0].x, vertices[0].y, vertices.back().x, vertices.back().y, inner_offset);
                first_point = Float2(vertices[0].x + d.x, vertices[0].y + d.y);
                move_to(points, first_point.x, first_point.y);
                for(usize i = 1; i < vertices.size(); ++i)
                {
                    usize draw_index = vertices.size() - i - 1;
                    usize first_vertex = draw_index == 0 ? vertices.size() - 1 : draw_index - 1;
                    d = calc_polyline_offset(vertices[draw_index + 1].x, vertices[draw_index + 1].y, 
                        vertices[draw_index].x, vertices[draw_index].y, vertices[first_vertex].x, vertices[first_vertex].y, inner_offset);
                    line_to(points, vertices[draw_index].x + d.x, vertices[draw_index].y + d.y);
                }
                line_to(points, first_point.x, first_point.y);
            }
            LUNA_VG_API void add_polyline(Vector<f32>& points, Span<const Float2U> vertices, f32 line_width, f32 line_offset)
            {
                if(vertices.size() < 2) return;
                // draw outside polygon.
                f32 half_line_width = line_width / 2.0f;
                f32 outer_offset = half_line_width + line_offset;
                Float2 first_d = normalize(Float2(vertices[1].x - vertices[0].x, vertices[1].y - vertices[0].y));
                first_d = Float2(-first_d.y, first_d.x);
                Float2 d = first_d * outer_offset;
                move_to(points, vertices[0].x + d.x, vertices[0].y + d.y);
                for(usize i = 1; i < vertices.size() - 1; ++i)
                {
                    d = calc_polyline_offset(vertices[i - 1].x, vertices[i - 1].y, vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y, outer_offset);
                    line_to(points, vertices[i].x + d.x, vertices[i].y + d.y);
                }
                d = normalize(Float2(vertices.back().x - vertices[vertices.size() - 2].x, vertices.back().y - vertices[vertices.size() - 2].y));
                d = Float2(-d.y, d.x);
                line_to(points, vertices.back().x + d.x * outer_offset, vertices.back().x + d.y * outer_offset);
                // draw inner polygon.
                f32 inner_offset = half_line_width - line_offset;
                line_to(points, vertices.back().x + -d.x * inner_offset, vertices.back().y + -d.y * inner_offset);
                for(usize i = 1; i < vertices.size() - 1; ++i)
                {
                    usize draw_index = vertices.size() - i - 1;
                    d = calc_polyline_offset(vertices[draw_index + 1].x, vertices[draw_index + 1].y, 
                        vertices[draw_index].x, vertices[draw_index].y, vertices[draw_index - 1].x, vertices[draw_index - 1].y, inner_offset);
                    line_to(points, vertices[draw_index].x + d.x, vertices[draw_index].y + d.y);
                }
                d = first_d * -inner_offset;
                line_to(points, vertices[0].x + d.x, vertices[0].y + d.y);
                d = first_d * outer_offset;
                line_to(points, vertices[0].x + d.x, vertices[0].y + d.y);
            }
        }
    }
}