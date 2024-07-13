/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Shape.hpp
* @author JXMaster
* @date 2023/9/26
*/
#pragma once
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Math/Vector.hpp>

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! @addtogroup VG
        //! @{
        
        //! The command code that begins one new path.
        //! @details This command takes 3 points: {COMMAND_MOVE_TO, X, Y}
        //! * X: The x coordinates of the initial position.
        //! * Y: The y coordinates of the initial position.
        //! 
        //! The former path will be closed when one begin command is detected.
        constexpr f32 COMMAND_MOVE_TO = 1.0f;
        //! The command code that draws one line from the last point to the specified point.
        //! @details This command takes 3 points: {COMMAND_LINE_TO, X, Y}
        //! * X: The x coordinates of the target position.
        //! * Y: The y coordinates of the target position.
        constexpr f32 COMMAND_LINE_TO = 2.0f;
        //! The command code that draws a quadratic Belzier curve to the specified point.
        //! @details This command takes 5 points: {COMMAND_CURVE_TO, CX, CY, X, Y}
        //! * CX: The x coordinates of the curve control point.
        //! * CY: The y coordinates of the curve control point.
        //! * X: The x coordinates of the target position.
        //! * Y: The y coordinates of the target position.
        constexpr f32 COMMAND_CURVE_TO = 3.0f;

        //! The command code that draws one circle part in the first quadrant.
        //! @details This command takes 4 points: {COMMAND_CIRCLE_Q1, R, BEGIN, END}
        //! * R: The radius of the circle.
        //! * BEGIN: The beginning angle of the circle part in degrees. The value should be in [0, 90].
        //! * END: The ending angle of the circle part in degrees. The value should be in [0, 90].
        //! @remark
        //! ```
        //!              90
        //!               y
        //!               ^
        //!               |
        //!       Q2      |        Q1
        //!               |
        //! 180 -------------------->x 0
        //!               |
        //!       Q3      |        Q4
        //!               |
        //!               |
        //!              270
        //! ```
        //! All circle drawing commands take three data points:
        //! * R: The radius of the circle.
        //! * BEGIN: The beginning angle of the circle in degrees.
        //! * END: The end angle of the circle in degrees.
        //! If the end angle is greater than the beginning angle, the circle is drawn counter-clockwisly,
        //! otherwise, the circle is drawn clockwisly.
        constexpr f32 COMMAND_CIRCLE_Q1 = 4.0f;
        //! The command code that draws one circle part in the second quadrant.
        //! @details This command takes 4 points: {COMMAND_CIRCLE_Q2, R, BEGIN, END}
        //! * R: The radius of the circle.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [90, 180].
        //! * END: The end angle of the circle in degrees. The value should be in [90, 180].
        //! See remarks of @ref COMMAND_CIRCLE_Q1 for details.
        constexpr f32 COMMAND_CIRCLE_Q2 = 5.0f;
        //! The command code that draws one circle part in the third quadrant.
        //! @details This command takes 4 points: {COMMAND_CIRCLE_Q3, R, BEGIN, END}
        //! * R: The radius of the circle.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [180, 270].
        //! * END: The end angle of the circle in degrees. The value should be in [180, 270].
        //! See remarks of @ref COMMAND_CIRCLE_Q1 for details.
        constexpr f32 COMMAND_CIRCLE_Q3 = 6.0f;
        //! The command code that draws one circle part in the fourth quadrant.
        //! @details This command takes 4 points: {COMMAND_CIRCLE_Q4, R, BEGIN, END}
        //! * R: The radius of the circle.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [270, 360].
        //! * END: The end angle of the circle in degrees. The value should be in [270, 360].
        //! See remarks of @ref COMMAND_CIRCLE_Q1 for details.
        constexpr f32 COMMAND_CIRCLE_Q4 = 7.0f;
        //! The command code that draws one axis-aligned ellipse part in the first quadrant.
        //! @details This command takes 5 points: {COMMAND_CIRCLE_Q2, RX, RY, BEGIN, END}
        //! * RX: The radius of the ellipse in X axis.
        //! * RY: The radius of the ellipse in Y axis.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [90, 180].
        //! * END: The end angle of the circle in degrees. The value should be in [90, 180].
        //! @remark 
        //! The axis-aligned ellipse drawing command is similar to circle drawing commands, except that
        //! the radius in X and Y axis can be set separately.
        //! 
        //! To draw one ellipse that is not axis-aligned, the user can rotate the draw vertex of the ellipse
        //! directly.
        constexpr f32 COMMAND_AXIS_ALIGNED_ELLIPSE_Q1 = 8.0f;
        //! The command code that draws one axis-aligned ellipse part in the second quadrant.
        //! @details This command takes 5 points: {COMMAND_CIRCLE_Q2, RX, RY, BEGIN, END}
        //! * RX: The radius of the ellipse in X axis.
        //! * RY: The radius of the ellipse in Y axis.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [90, 180].
        //! * END: The end angle of the circle in degrees. The value should be in [90, 180].
        //! See remarks of @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q1 for details.
        constexpr f32 COMMAND_AXIS_ALIGNED_ELLIPSE_Q2 = 9.0f;
        //! The command code that draws one axis-aligned ellipse part in the third quadrant.
        //! @details This command takes 5 points: {COMMAND_CIRCLE_Q2, RX, RY, BEGIN, END}
        //! * RX: The radius of the ellipse in X axis.
        //! * RY: The radius of the ellipse in Y axis.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [90, 180].
        //! * END: The end angle of the circle in degrees. The value should be in [90, 180].
        //! See remarks of @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q1 for details.
        constexpr f32 COMMAND_AXIS_ALIGNED_ELLIPSE_Q3 = 10.0f;
        //! The command code that draws one axis-aligned ellipse part in the fourth quadrant.
        //! @details This command takes 5 points: {COMMAND_CIRCLE_Q2, RX, RY, BEGIN, END}
        //! * RX: The radius of the ellipse in X axis.
        //! * RY: The radius of the ellipse in Y axis.
        //! * BEGIN: The beginning angle of the circle in degrees. The value should be in [90, 180].
        //! * END: The end angle of the circle in degrees. The value should be in [90, 180].
        //! See remarks of @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q1 for details.
        constexpr f32 COMMAND_AXIS_ALIGNED_ELLIPSE_Q4 = 11.0f;

        namespace ShapeBuilder
        {
            //! @addtogroup ShapeBuilder A collection of functions that help generating shape command points.
            //! @{
            
            //! Adds one @ref COMMAND_MOVE_TO command to shape data points.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] x The x coordinates of the initial position.
            //! @param[in] y The y coordinates of the initial position.
            inline void move_to(Vector<f32>& points, f32 x, f32 y) { points.insert(points.end(), { COMMAND_MOVE_TO, x, y }); }
            //! Adds one @ref COMMAND_LINE_TO command to shape data points.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] x The x coordinates of the target position.
            //! @param[in] y The y coordinates of the target position.
            inline void line_to(Vector<f32>& points, f32 x, f32 y) { points.insert(points.end(), { COMMAND_LINE_TO, x, y }); }
            //! Adds one @ref COMMAND_CURVE_TO command to shape data points.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] cx The x coordinates of the curve control point.
            //! @param[in] cy The y coordinates of the curve control point.
            //! @param[in] x The x coordinates of the target position.
            //! @param[in] y The y coordinates of the target position.
            inline void curve_to(Vector<f32>& points, f32 cx, f32 cy, f32 x, f32 y) { points.insert(points.end(), { COMMAND_CURVE_TO, cx, cy, x, y }); }
            //! Adds commands to draw one circle part to shape data points.
            //! @details If the draw circle part overlaps multiple quadrants, this function
            //! seprates the circle part to multiple @ref COMMAND_CIRCLE_Q1, @ref COMMAND_CIRCLE_Q2,
            //! @ref COMMAND_CIRCLE_Q3 and @ref COMMAND_CIRCLE_Q4 commands automatically.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] radius The radius of the circle.
            //! @param[in] begin The beginning angle of the circle in degrees.
            //! @param[in] end The end angle of the circle in degrees.
            //! If the end angle is greater than the beginning angle, the circle is drawn counter-clockwisly,
            //! otherwise, the circle is drawn clockwisly.
            LUNA_VG_API void circle_to(Vector<f32>& points, f32 radius, f32 begin, f32 end);
            //! Adds commands to draw one axis aligned ellipse part to shape data points.
            //! @details If the draw ellipse part overlaps multiple quadrants, this function
            //! seprates the ellipse part to multiple @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q1, @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q2,
            //! @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q3 and @ref COMMAND_AXIS_ALIGNED_ELLIPSE_Q4 commands automatically.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] radius_x The radius of the ellipse in X axis.
            //! @param[in] begin The beginning angle of the ellipse in degrees.
            //! @param[in] end The end angle of the ellipse in degrees.
            //! If the end angle is greater than the beginning angle, the ellipse is drawn counter-clockwisly,
            //! otherwise, the ellipse is drawn clockwisly.
            LUNA_VG_API void axis_aligned_ellipse_to(Vector<f32>& points, f32 radius_x, f32 radius_y, f32 begin, f32 end);
            //! Adds commands to draw one filled axis-aligned rectangle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] min_x The minimum x coordinates of the rectangle.
            //! @param[in] min_y The minimum y coordinates of the rectangle.
            //! @param[in] max_x The maximum x coordinates of the rectangle.
            //! @param[in] max_y The maximum y coordinates of the rectangle.
            LUNA_VG_API void add_rectangle_filled(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y);
            //! Adds commands to draw one bordered axis-aligned rectangle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] min_x The minimum x coordinates of the rectangle.
            //! @param[in] min_y The minimum y coordinates of the rectangle.
            //! @param[in] max_x The maximum x coordinates of the rectangle.
            //! @param[in] max_y The maximum y coordinates of the rectangle.
            //! @param[in] border_width The width of the border line.
            //! @param[in] border_offset The offset of the border line relative to the rectangle border. Positive value
            //! makes the border line move outside of the rectangle, while negative value makes the border line move inside of the rectangle.
            LUNA_VG_API void add_rectangle_bordered(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 border_width, f32 border_offset = 0.0f);
            //! Adds commands to draw one line.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] p1_x The x coordinates of the first point.
            //! @param[in] p1_y The y coordinates of the first point.
            //! @param[in] p2_x The x coordinates of the second point.
            //! @param[in] p2_y The y coordinates of the second point.
            //! @param[in] width The line width.
            //! @param[in] offset The offset of the drawn line relative to the original line formed by two points, in the perpendicular direction
            //! of the line direction.
            LUNA_VG_API void add_line(Vector<f32>& points, f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 width, f32 offset = 0.0f);
            //! Adds commands to draw one filled axis-aligned rounded rectangle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] min_x The minimum x coordinates of the rectangle.
            //! @param[in] min_y The minimum y coordinates of the rectangle.
            //! @param[in] max_x The maximum x coordinates of the rectangle.
            //! @param[in] max_y The maximum y coordinates of the rectangle.
            //! @param[in] radius The radius of round edges.
            LUNA_VG_API void add_rounded_rectangle_filled(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius);
            //! Adds commands to draw one bordered axis-aligned rounded rectangle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] min_x The minimum x coordinates of the rectangle.
            //! @param[in] min_y The minimum y coordinates of the rectangle.
            //! @param[in] max_x The maximum x coordinates of the rectangle.
            //! @param[in] max_y The maximum y coordinates of the rectangle.
            //! @param[in] radius The radius of round edges.
            //! @param[in] border_width The width of the border line.
            //! @param[in] border_offset The offset of the border line relative to the rectangle border. Positive value
            //! makes the border line move outside of the rectangle, while negative value makes the border line move inside of the rectangle.
            LUNA_VG_API void add_rounded_rectangle_bordered(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius, f32 border_width, f32 border_offset = 0.0f);
            //! Adds commands to draw one filled circle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] center_x The x coordinates of the circle center.
            //! @param[in] center_y The y coordinates of the circle center.
            //! @param[in] radius The circle radius.
            LUNA_VG_API void add_circle_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius);
            //! Adds commands to draw one bordered circle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] center_x The x coordinates of the circle center.
            //! @param[in] center_y The y coordinates of the circle center.
            //! @param[in] radius The circle radius.
            //! @param[in] border_width The width of the border line.
            //! @param[in] border_offset The offset of the border line relative to the circle border. Positive value
            //! makes the border line move outside of the circle, while negative value makes the border line move inside of the circle.
            LUNA_VG_API void add_circle_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 border_width, f32 border_offset = 0.0f);
            //! Adds commands to draw one filled arc.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] center_x The x coordinates of the arc center.
            //! @param[in] center_y The y coordinates of the arc center.
            //! @param[in] radius The arc radius.
            //! @param[in] begin_angle The angle of the beginning point of the arc in degrees.
            //! @param[in] end_angle The angle of the ending point of the arc in degrees.
            LUNA_VG_API void add_arc_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 begin_angle, f32 end_angle);
            //! Adds commands to draw one bordered arc.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] center_x The x coordinates of the arc center.
            //! @param[in] center_y The y coordinates of the arc center.
            //! @param[in] radius The arc radius.
            //! @param[in] begin_angle The angle of the beginning point of the arc in degrees.
            //! @param[in] end_angle The angle of the ending point of the arc in degrees.
            //! @param[in] border_width The width of the border line.
            //! @param[in] border_offset The offset of the border line relative to the arc border. Positive value
            //! makes the border line move outside of the arc, while negative value makes the border line move inside of the arc.
            LUNA_VG_API void add_arc_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 begin_angle, f32 end_angle, f32 border_width, f32 border_offset = 0.0f);
            //! Adds commands to draw one filled axis aligned ellipse.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] center_x The x coordinates of the ellipse center.
            //! @param[in] center_y The y coordinates of the ellipse center.
            //! @param[in] radius_x The ellipse radius in X axis.
            //! @param[in] radius_y The ellipse radius in Y axis.
            LUNA_VG_API void add_axis_aligned_ellipse_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius_x, f32 radius_y);
            //! Adds commands to draw one bordered axis aligned ellipse.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] center_x The x coordinates of the ellipse center.
            //! @param[in] center_y The y coordinates of the ellipse center.
            //! @param[in] radius_x The ellipse radius in X axis.
            //! @param[in] radius_y The ellipse radius in Y axis.
            //! @param[in] border_width The width of the border line.
            //! @param[in] border_offset The offset of the border line relative to the ellipse border. Positive value
            //! makes the border line move outside of the ellipse, while negative value makes the border line move inside of the ellipse.
            LUNA_VG_API void add_axis_aligned_ellipse_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius_x, f32 radius_y, f32 border_width, f32 border_offset = 0.0f);
            //! Adds commands to draw one filled triangle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] x1 The x coordinates of the first triangle point.
            //! @param[in] y1 The y coordinates of the first triangle point.
            //! @param[in] x2 The x coordinates of the second triangle point.
            //! @param[in] y2 The y coordinates of the second triangle point.
            //! @param[in] x3 The x coordinates of the third triangle point.
            //! @param[in] y3 The y coordinates of the third triangle point.
            LUNA_VG_API void add_triangle_filled(Vector<f32>& points, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
            //! Adds commands to draw one bordered triangle.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] x1 The x coordinates of the first triangle point.
            //! @param[in] y1 The y coordinates of the first triangle point.
            //! @param[in] x2 The x coordinates of the second triangle point.
            //! @param[in] y2 The y coordinates of the second triangle point.
            //! @param[in] x3 The x coordinates of the third triangle point.
            //! @param[in] y3 The y coordinates of the third triangle point.
            //! @param[in] border_width The width of the border line.
            //! @param[in] border_offset The offset of the border line relative to the triangle border. Positive value
            //! makes the border line move outside of the triangle, while negative value makes the border line move inside of the triangle.
            LUNA_VG_API void add_triangle_bordered(Vector<f32>& points, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 border_width, f32 border_offset = 0.0f);
            
            //! Adds one closed filled polygon.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] vertices The vertices of the polygon.
            LUNA_VG_API void add_polygon_filled(Vector<f32>& points, Span<const Float2U> vertices);

            //! Adds one closed bordered polygon.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] vertices The vertices of the polygon.
            //! @param[in] border_width The width of the border.
            //! @param[in] border_offset The offset of the border.
            //! If the polygon is winded clockwisly, positive value makes the border line move outside of the polygon, while negative value makes the border line move inside of the polygon.
            LUNA_VG_API void add_polygon_bordered(Vector<f32>& points, Span<const Float2U> vertices, f32 border_width, f32 border_offset = 0.0f);

            //! Adds one line that is compose by multiple vertices connected by line strips.
            //! @param[in] points The shape data point buffer to add command to.
            //! @param[in] vertices The vertices of the polyline.
            //! @param[in] line_width The width of the polyline.
            //! @param[in] line_offset The offset of the polyline.
            LUNA_VG_API void add_polyline(Vector<f32>& points, Span<const Float2U> vertices, f32 line_width, f32 line_offset = 0.0f);

            //! @}
        }

        //! @}
    }
}