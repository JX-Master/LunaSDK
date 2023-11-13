/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Controller.cpp
* @author JXMaster
* @date 2023/11/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Controller.hpp"
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "User32.lib")
#include <Luna/Runtime/Math/Math.hpp>

namespace Luna
{
    namespace HID
    {
        LUNA_HID_API bool supports_controller()
        {
            return true;
        }
        inline void normalize_thumb(f32 x, f32 y, f32 deadzone, f32& outx, f32& outy)
		{
			f32 l = sqrtf(x * x + y * y);
			l = max(l, F32_EPSILON);	// prevent dividing by 0.
			f32 lc = max(l, deadzone);
			lc -= deadzone;
			f32 scalef = lc / l / (32767.0f - deadzone);
			outx = x * scalef;
			outy = y * scalef;
		}
		inline void normalize_trigger(f32 inv, f32 deadzone, f32& outv)
		{
			constexpr f32 max_trigger = 255.0f;
			inv = max(inv, deadzone);
			outv = (inv - deadzone) / (max_trigger - deadzone);
		}
		LUNA_HID_API ControllerInputState get_controller_state(u32 index)
		{
			ControllerInputState ret;
			memzero(&ret, sizeof(ControllerInputState));
			if (index >= XUSER_MAX_COUNT)
			{
				return ret;
			}
			XINPUT_STATE state;
			DWORD dw = XInputGetState(index, &state);
			if (dw == ERROR_SUCCESS)
			{
				/*
				#define XINPUT_GAMEPAD_DPAD_UP          0x0001
				#define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
				#define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
				#define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
				#define XINPUT_GAMEPAD_START            0x0010
				#define XINPUT_GAMEPAD_BACK             0x0020
				#define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
				#define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
				#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
				#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
				#define XINPUT_GAMEPAD_A                0x1000
				#define XINPUT_GAMEPAD_B                0x2000
				#define XINPUT_GAMEPAD_X                0x4000
				#define XINPUT_GAMEPAD_Y                0x8000
				*/
				ret.connected = true;
				WORD buttons = state.Gamepad.wButtons;
				if (buttons & XINPUT_GAMEPAD_DPAD_UP) set_flags(ret.buttons, ControllerButton::up);
				if (buttons & XINPUT_GAMEPAD_DPAD_DOWN) set_flags(ret.buttons, ControllerButton::down);
				if (buttons & XINPUT_GAMEPAD_DPAD_LEFT) set_flags(ret.buttons, ControllerButton::left);
				if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) set_flags(ret.buttons, ControllerButton::right);
				if (buttons & XINPUT_GAMEPAD_START) set_flags(ret.buttons, ControllerButton::rspecial);
				if (buttons & XINPUT_GAMEPAD_BACK) set_flags(ret.buttons, ControllerButton::lspecial);
				if (buttons & XINPUT_GAMEPAD_LEFT_THUMB) set_flags(ret.buttons, ControllerButton::lthumb);
				if (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) set_flags(ret.buttons, ControllerButton::rthumb);
				if (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) set_flags(ret.buttons, ControllerButton::lb);
				if (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) set_flags(ret.buttons, ControllerButton::rb);
				if (buttons & XINPUT_GAMEPAD_A) set_flags(ret.buttons, ControllerButton::a);
				if (buttons & XINPUT_GAMEPAD_B) set_flags(ret.buttons, ControllerButton::b);
				if (buttons & XINPUT_GAMEPAD_X) set_flags(ret.buttons, ControllerButton::x);
				if (buttons & XINPUT_GAMEPAD_Y) set_flags(ret.buttons, ControllerButton::y);
				if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) set_flags(ret.buttons, ControllerButton::lt);
				if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) set_flags(ret.buttons, ControllerButton::rt);
				normalize_thumb(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, ret.axis_lx, ret.axis_ly);
				normalize_thumb(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, ret.axis_rx, ret.axis_ry);
				normalize_trigger(state.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, ret.axis_lt);
				normalize_trigger(state.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, ret.axis_rt);
			}
			return ret;
		}
		RV set_controller_state(u32 index, const ControllerOutputState& state)
		{
			if (index >= XUSER_MAX_COUNT)
			{
				return BasicError::bad_platform_call();
			}
			XINPUT_VIBRATION vib;
			f32 lb = clamp(state.left_vibration, 0.0f, 1.0f);
			f32 rb = clamp(state.right_vibration, 0.0f, 1.0f);
			vib.wLeftMotorSpeed = (WORD)(lb * 65535.0f);
			vib.wRightMotorSpeed = (WORD)(rb * 65535.0f);
			DWORD r = XInputSetState(index, &vib);
			return (r == ERROR_SUCCESS) ? RV() : BasicError::bad_platform_call();
		}
    }
}