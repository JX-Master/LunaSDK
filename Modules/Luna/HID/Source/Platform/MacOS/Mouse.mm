/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mouse.mm
* @author JXMaster
* @date 2023/11/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Mouse.hpp"
#import <Cocoa/Cocoa.h>

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
            @autoreleasepool
            {
                NSUInteger buttons = [NSEvent pressedMouseButtons];
                switch(mouse_button)
                {
                    case MouseButton::left:
                        return (buttons & (1 << 0)) != 0;
                    case MouseButton::right:
                        return (buttons & (1 << 1)) != 0;
                    case MouseButton::middle:
                        return (buttons & (1 << 2)) != 0;
                    case MouseButton::function1:
                        return (buttons & (1 << 3)) != 0;
                    case MouseButton::function2:
                        return (buttons & (1 << 4)) != 0;
                    default: break;
                }
            }
            return false;
        }
        LUNA_HID_API Int2U get_mouse_pos()
        {
            @autoreleasepool
            {
                CGEventRef mouse_event = CGEventCreate(NULL);
                CGPoint mouse_position = CGEventGetLocation(mouse_event);
                CFRelease(mouse_event);
                return Int2U(mouse_position.x, mouse_position.y);
            }
        }
        LUNA_HID_API RV set_mouse_pos(i32 x, i32 y)
        {
            @autoreleasepool
            {
                CGPoint new_mouse_location = CGPointMake(x, y);
                CGError error = CGWarpMouseCursorPosition(new_mouse_location);
                if(error != kCGErrorSuccess)
                {
                    if(error == kCGErrorIllegalArgument) return BasicError::bad_arguments();
                    if(error == kCGErrorNotImplemented) return BasicError::not_supported();
                    return BasicError::bad_platform_call();
                }
            }
            return ok;
        }

    }
}