/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WindowsDevice.cpp
* @author JXMaster
* @date 2022/4/1
*/
#pragma once
#include "../../HID.hpp"
#include "../../../HID.hpp"
#include "../../../Mouse.hpp"
#include "../../../Keyboard.hpp"
#include "../../../Controller.hpp"
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "User32.lib")
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/Runtime/Ref.hpp>

namespace Luna
{
	namespace HID
	{
		struct WindowsDevice : IMouse, IKeyboard, IController
		{
			lustruct("HID::WindowsDevice", "{BCC4DFB6-7E68-4E45-A1D1-560EE393E5F4}");
			luiimpl();

			virtual Int2U get_cursor_pos() override;
			virtual bool get_button_state(MouseButton mouse_button) override;
			virtual RV set_cursor_pos(i32 x, i32 y) override;
			virtual bool get_key_state(KeyCode key) override;
			virtual ControllerInputState get_state(u32 index) override;
			virtual RV set_state(u32 index, const ControllerOutputState& state) override;
		};

		Ref<WindowsDevice> g_device;

		Int2U WindowsDevice::get_cursor_pos()
		{
			POINT p;
			::GetCursorPos(&p);
			return Int2U(p.x, p.y);
		}
		bool WindowsDevice::get_button_state(MouseButton mouse_button)
		{
			int key;
			switch (mouse_button)
			{
			case MouseButton::left:
				key = VK_LBUTTON;
				break;
			case MouseButton::right:
				key = VK_RBUTTON;
				break;
			case MouseButton::middle:
				key = VK_MBUTTON;
				break;
			case MouseButton::function1:
				key = VK_XBUTTON1;
				break;
			case MouseButton::function2:
				key = VK_XBUTTON2;
				break;
			default:
				return false;
			}
			return (GetAsyncKeyState(key) & 0x8000) ? true : false;
		}
		RV WindowsDevice::set_cursor_pos(i32 x, i32 y)
		{
			BOOL r = ::SetCursorPos(x, y);
			return r ? RV() : BasicError::bad_platform_call();
		}
		static int map_virtual_key(KeyCode key)
		{
			if ((u16)key >= (u16)KeyCode::num0 && (u16)key <= (u16)KeyCode::num9)
			{
				return (u16)key - (u16)KeyCode::num0 + 0x30;
			}
			if ((u16)key >= (u16)KeyCode::a && (u16)key <= (u16)KeyCode::z)
			{
				return (u16)key - (u16)KeyCode::a + 0x41;
			}
			if ((u16)key >= (u16)KeyCode::f1 && (u16)key <= (u16)KeyCode::f12)
			{
				return (u16)key - (u16)KeyCode::f1 + VK_F1;
			}
			if ((u16)key >= (u16)KeyCode::numpad0 && (u16)key <= (u16)KeyCode::numpad9)
			{
				return (u16)key - (u16)KeyCode::numpad0 + VK_NUMPAD0;
			}
			switch (key)
			{
			case KeyCode::esc:				return VK_ESCAPE;
			case KeyCode::grave:			return VK_OEM_3;
			case KeyCode::equal:			return VK_OEM_PLUS;
			case KeyCode::minus:			return VK_OEM_MINUS;
			case KeyCode::backspace:		return VK_BACK;
			case KeyCode::tab:				return VK_TAB;
			case KeyCode::caps_lock:		return VK_CAPITAL;
			case KeyCode::enter:			return VK_RETURN;
			case KeyCode::ctrl:				return VK_CONTROL;
			case KeyCode::l_ctrl:			return VK_LCONTROL;
			case KeyCode::r_ctrl:			return VK_RCONTROL;
			case KeyCode::shift:			return VK_SHIFT;
			case KeyCode::l_shift:			return VK_LSHIFT;
			case KeyCode::r_shift:			return VK_RSHIFT;
			case KeyCode::menu:				return VK_MENU;
			case KeyCode::l_menu:			return VK_LMENU;
			case KeyCode::r_menu:			return VK_RMENU;
			case KeyCode::l_system:			return VK_LWIN;
			case KeyCode::r_system:			return VK_RWIN;
			case KeyCode::apps:				return VK_APPS;
			case KeyCode::spacebar:			return VK_SPACE;
			case KeyCode::l_branket:		return VK_OEM_4;
			case KeyCode::r_branket:		return VK_OEM_6;
			case KeyCode::backslash:		return VK_OEM_5;
			case KeyCode::semicolon:		return VK_OEM_1;
			case KeyCode::quote:			return VK_OEM_7;
			case KeyCode::comma:			return VK_OEM_COMMA;
			case KeyCode::period:			return VK_OEM_PERIOD;
			case KeyCode::slash:			return VK_OEM_2;
			case KeyCode::print_screen:		return VK_SNAPSHOT;
			case KeyCode::scroll_lock:		return VK_SCROLL;
			case KeyCode::pause:			return VK_PAUSE;
			case KeyCode::insert:			return VK_INSERT;
			case KeyCode::home:				return VK_HOME;
			case KeyCode::page_up:			return VK_PRIOR;
			case KeyCode::page_down:		return VK_NEXT;
			case KeyCode::del:				return VK_DELETE;
			case KeyCode::end:				return VK_END;
			case KeyCode::left:				return VK_LEFT;
			case KeyCode::up:				return VK_UP;
			case KeyCode::right:			return VK_RIGHT;
			case KeyCode::down:				return VK_DOWN;
			case KeyCode::num_lock:			return VK_NUMLOCK;
			case KeyCode::numpad_decimal:	return VK_DECIMAL;
			case KeyCode::numpad_add:		return VK_ADD;
			case KeyCode::numpad_subtract:	return VK_SUBTRACT;
			case KeyCode::numpad_multiply:	return VK_MULTIPLY;
			case KeyCode::numpad_divide:	return VK_DIVIDE;
			default: lupanic(); return 0;
			}
		}

		bool WindowsDevice::get_key_state(KeyCode key)
		{
			return (GetAsyncKeyState(map_virtual_key(key) & 0x8000)) ? true : false;
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
		ControllerInputState WindowsDevice::get_state(u32 index)
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
		RV WindowsDevice::set_state(u32 index, const ControllerOutputState& state)
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

		RV request_windows_device(void* userdata, const Guid& iid, object_t* out_device_object)
		{
			object_t obj = g_device.object();
			object_retain(obj);
			*out_device_object = obj;
			return ok;
		}

		RV register_platform_devices()
		{
			register_boxed_type<WindowsDevice>();
			impl_interface_for_type<WindowsDevice, IMouse, IKeyboard, IController>();
			g_device = new_object<WindowsDevice>();
			DeviceDesc desc;
			desc.userdata = nullptr;
			desc.supported_iids = { IMouse::__guid, IKeyboard::__guid, IController::__guid };
			desc.on_request_device = request_windows_device;
			desc.on_unregister = nullptr;
			return register_device("WindowsDevice", desc);
		}

		void unregister_platform_devices()
		{
			unregister_device("WindowsDevice");
			g_device.reset();
		}
	}
}