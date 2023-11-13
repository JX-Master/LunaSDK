/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Keyboard.cpp
* @author JXMaster
* @date 2023/11/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Keyboard.hpp"

namespace Luna
{
    namespace HID
    {
        LUNA_HID_API bool supports_keyboard()
        {
            return true;
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
		LUNA_HID_API bool get_key_state(KeyCode key)
		{
			return (GetAsyncKeyState(map_virtual_key(key)) & 0x8000) ? true : false;
		}
    }
}