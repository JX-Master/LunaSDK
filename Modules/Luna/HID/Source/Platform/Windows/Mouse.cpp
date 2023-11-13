/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mouse.cpp
* @author JXMaster
* @date 2023/11/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Mouse.hpp"

namespace Luna
{
    namespace HID
    {
        LUNA_HID_API bool supports_mouse()
		{
			return true;
		}
		LUNA_HID_API bool get_mouse_button_state(MouseButton mouse_button)
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
        LUNA_HID_API Int2U get_mouse_pos()
		{
			POINT p;
			::GetCursorPos(&p);
			return Int2U(p.x, p.y);
		}
		LUNA_HID_API RV set_mouse_pos(i32 x, i32 y)
		{
			BOOL r = ::SetCursorPos(x, y);
			return r ? RV() : BasicError::bad_platform_call();
		}
    }
}