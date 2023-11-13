/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mouse.hpp
* @author JXMaster
* @date 2019/7/24
*/
#pragma once
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>
#include "KeyCode.hpp"

#ifndef LUNA_HID_API
#define LUNA_HID_API
#endif

namespace Luna
{
	namespace HID
	{
		//! Checks if mouse input is supported on the current platform.
		LUNA_HID_API bool supports_mouse();

		//! Checks if the specified mouse button is pressed.
		//! @return `true` if the mouse button is down, returns `false` otherwise.
		//! If mouse input is not supported, returns `false` always.
		LUNA_HID_API bool get_mouse_button_state(MouseButton mouse_button);

		//! Get the position of the mouse cursor in screen space.
		//! @return The position of the mouse cursor in screen space.
		//! If screen cursor API is not supported, returns `(0, 0)` always.
		LUNA_HID_API Int2U get_mouse_pos();

		//! Sets the OS mouse cursor position. The position is based on the screen coordinates.
		//! This only works for platforms that support mouse input.
		LUNA_HID_API RV set_mouse_pos(i32 x, i32 y);
	}
}