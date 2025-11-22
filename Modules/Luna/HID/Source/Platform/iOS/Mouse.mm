/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mouse.mm
* @author JXMaster
* @date 2025/11/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Mouse.hpp"

namespace Luna
{
    namespace HID
    {
        // Traditional mouse input is not generally available on iOS devices.
        // Pointer / touch input is delivered via window events instead.

        LUNA_HID_API bool supports_mouse()
        {
            return false;
        }

        LUNA_HID_API bool get_mouse_button_state(MouseButton mouse_button)
        {
            (void)mouse_button;
            return false;
        }

        LUNA_HID_API Int2U get_mouse_pos()
        {
            // No global mouse cursor on iOS; return (0, 0).
            return Int2U(0, 0);
        }

        LUNA_HID_API RV set_mouse_pos(i32 x, i32 y)
        {
            (void)x;
            (void)y;
            return BasicError::not_supported();
        }
    }
}


