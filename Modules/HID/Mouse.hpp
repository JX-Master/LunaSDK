/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mouse.hpp
* @author JXMaster
* @date 2019/7/24
*/
#pragma once
#include <Runtime/Math/Vector.hpp>
#include <Runtime/Interface.hpp>
#include <Runtime/Result.hpp>
#include "KeyCode.hpp"

namespace Luna
{
	namespace HID
	{
		//! @interface IMouse
		//! Represents the mouse input device.
		struct IMouse : virtual Interface
		{
			luiid("{7d3a9e2e-eff4-4341-99e0-78ef9c36f7dd}");

			//! Checks if the specified mouse button is pressed.
			//! @return `true` if the mouse button is down, returns `false` otherwise.
			virtual bool get_button_state(MouseButton mouse_button) = 0;

			//! Get the position of the mouse cursor in screen space.
			//! @return The position of the mouse cursor in screen space.
			virtual Int2U get_cursor_pos() = 0;

			//! Sets the OS mouse cursor position. The position is based on the screen coordinates.
			//! This only works for Desktop-based system.
			virtual RV set_cursor_pos(i32 x, i32 y) = 0;
		};
	}
}