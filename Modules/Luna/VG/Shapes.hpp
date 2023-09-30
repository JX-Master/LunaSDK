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

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! Begins one new path.
		//! 0: The x coordinates of the initial position.
		//! 1: The y coordinates of the initial position.
		//! The former path will be closed when one begin command is detected.
		constexpr f32 COMMAND_MOVE_TO = 1.0f;
		//! Draws one line to the specified point.
		//! Data points:
		//! 0: The x coordinates of the target position.
		//! 1: The y coordinates of the target position.
		constexpr f32 COMMAND_LINE_TO = 2.0f;
		//! Draws a quadratic Belzier curve to the specified point.
		//! Data points:
		//! 0: The x coordinates of the curve control point.
		//! 1: The y coordinates of the curve control point.
		//! 2: The x coordinates of the target position.
		//! 3: The y coordinates of the target position.
		constexpr f32 COMMAND_CURVE_TO = 3.0f;

		/*
			Circle drawing commands.
					 90
					  y
					  ^
					  |
				Q2	  |		Q1
					  |
		180	-------------------->x 0
					  |
				Q3	  |		Q4
					  |
					  |
					 270

			All circle drawing commands take three data points:
			0: The radius of the circle.
			1: The beginning angle of the circle in degrees.
			2: The end angle of the circle in degrees.
			If the end angle is greater than the beginning angle, the circle is drawn counter-clockwisly,
		    otherwise, the circle is drawn clockwisly.
		*/

		//! Draws a circle part in the first quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [0, 90].
		//! 2: The end angle of the circle in degrees. The value should be in [0, 90].
		constexpr f32 COMMAND_CIRCLE_Q1 = 4.0f;
		//! Draws a circle part in the second quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [90, 180].
		//! 2: The end angle of the circle in degrees. The value should be in [90, 180].
		constexpr f32 COMMAND_CIRCLE_Q2 = 5.0f;
		//! Draws a circle part in the third quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [180, 270].
		//! 2: The end angle of the circle in degrees. The value should be in [180, 270].
		constexpr f32 COMMAND_CIRCLE_Q3 = 6.0f;
		//! Draws a circle part in the fourth quadrant.
		//! Data points:
		//! 0: The radius of the circle.
		//! 1: The beginning angle of the circle in degrees. The value should be in [270, 360].
		//! 2: The end angle of the circle in degrees. The value should be in [270, 360].
		constexpr f32 COMMAND_CIRCLE_Q4 = 7.0f;

        namespace ShapeBuilder
        {
            inline void move_to(Vector<f32>& points, f32 x, f32 y) { points.insert(points.end(), { COMMAND_MOVE_TO, x, y }); }
			inline void line_to(Vector<f32>& points, f32 x, f32 y) { points.insert(points.end(), { COMMAND_LINE_TO, x, y }); }
			inline void curve_to(Vector<f32>& points, f32 cx, f32 cy, f32 x, f32 y) { points.insert(points.end(), { COMMAND_CURVE_TO, cx, cy, x, y }); }
            LUNA_VG_API void circle_to(Vector<f32>& points, f32 radius, f32 begin, f32 end);
            LUNA_VG_API void add_rectangle_filled(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y);
            LUNA_VG_API void add_rectangle_bordered(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 border_width, f32 border_offset = 0.0f);
            LUNA_VG_API void add_line(Vector<f32>& points, f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 border_width, f32 border_offset = 0.0f);
            LUNA_VG_API void add_rounded_rectangle_filled(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius);
            LUNA_VG_API void add_rounded_rectangle_bordered(Vector<f32>& points, f32 min_x, f32 min_y, f32 max_x, f32 max_y, f32 radius, f32 border_width, f32 border_offset = 0.0f);
			LUNA_VG_API void add_circle_filled(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius);
			LUNA_VG_API void add_circle_bordered(Vector<f32>& points, f32 center_x, f32 center_y, f32 radius, f32 border_width, f32 border_offset = 0.0f);
        }
    }
}