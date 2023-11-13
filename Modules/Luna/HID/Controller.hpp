/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Controller.hpp
* @author JXMaster
* @date 2019/7/25
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_HID_API
#define LUNA_HID_API
#endif

namespace Luna
{
	namespace HID
	{
		enum class ControllerButton : u16
		{
			none		= 0x0000,
			lthumb		= 0x0001,	// Left thumb button.
			rthumb		= 0x0002,	// Right thumb button.
			up			= 0x0004,	// Up button.
			down		= 0x0008,	// Down button.
			left		= 0x0010,	// Left button.
			right		= 0x0020,	// Right button.
			a			= 0x0040,	// A button.
			b			= 0x0080,	// B button.
			x			= 0x0100,	// X button.
			y			= 0x0200,	// Y button.
			lb			= 0x0400,	// Left shoulder button. LB for XBOX controller, L for Nintendo controller.
			rb			= 0x0800,	// Right shoulder button. RB for XBOX controller, R for Nintendo controller.
			lt			= 0x1000,	// Left trigger button. LT for XBOX controller, ZL for Nintendo controller.
			rt			= 0x2000,	// Right trigger button. RT for XBOX controller, ZR for Nintendo controller.
			lspecial	= 0x4000,	// Left special button.
			rspecial	= 0x8000	// Right special button.
		};

		struct ControllerInputState
		{
			//! Whether this device is connected and the state is valid.
			bool connected;
			//! A combination of bits to represent the pressed state of each button (1 for pressed, 0 for not pressed).
			ControllerButton buttons;
			//! The x axis for left pad, mapped to [-1, 1].
			f32 axis_lx;
			//! The y axis for left pad, mapped to [-1, 1].
			f32 axis_ly;
			//! The x axis for right pad, mapped to [-1, 1].
			f32 axis_rx;
			//! The y axis for right pad, mapped to [-1, 1].
			f32 axis_ry;
			//! The left trigger value, mapped to [0, 1]. For non-linear controllers, the value is either 0 or 1.
			f32 axis_lt;
			//! The right trigger value, mapped to [0, 1]. For non-linear controllers, the value is either 0 or 1.
			f32 axis_rt;
		};

		struct ControllerOutputState
		{
			//! The vibration level for left vibration motor, mapped to [0, 1].
			f32 left_vibration;
			//! The vibration level for right vibration motor, mapped to [0, 1].
			f32 right_vibration;
		};

		//! Checks if game controller input is supported on the current platform.
		LUNA_HID_API bool supports_controller();

		//! Fetches the input state of the specified controller.
		//! @param[in] index The 0-based index of controller.
		//! @return Returns the state of the specified controller.
		//! If controller is not supported on this platform, returns one structure with all values set to 0.
		LUNA_HID_API ControllerInputState get_controller_state(u32 index);

		//! Sets the output state of the specified controller.
		//! @param[in] index The 0-based index of controller.
		//! @param[in] state The state to set.
		LUNA_HID_API RV set_controller_state(u32 index, const ControllerOutputState& state);
	}
}