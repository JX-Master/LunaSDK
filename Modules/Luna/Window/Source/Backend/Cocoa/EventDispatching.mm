/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventDispatching.mm
* @author JXMaster
* @date 2025/10/5
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../../Cocoa/EventHandling.hpp"
#include "Window.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/HID/KeyCode.hpp>
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#include <Carbon/Carbon.h>

namespace Luna
{
    namespace Window
    {
        inline HID::KeyCode translate_key(CGKeyCode key)
        {
            using namespace HID;
            switch(key)
            {
                case kVK_Escape: return KeyCode::esc;
                case kVK_F1: return KeyCode::f1;
                case kVK_F2: return KeyCode::f2;
                case kVK_F3: return KeyCode::f3;
                case kVK_F4: return KeyCode::f4;
                case kVK_F5: return KeyCode::f5;
                case kVK_F6: return KeyCode::f6;
                case kVK_F7: return KeyCode::f7;
                case kVK_F8: return KeyCode::f8;
                case kVK_F9: return KeyCode::f9;
                case kVK_F10: return KeyCode::f10;
                case kVK_F11: return KeyCode::f11;
                case kVK_F12: return KeyCode::f12;
                case kVK_ANSI_Grave: return KeyCode::grave;
                case kVK_ANSI_0: return KeyCode::num0;
                case kVK_ANSI_1: return KeyCode::num1;
                case kVK_ANSI_2: return KeyCode::num2;
                case kVK_ANSI_3: return KeyCode::num3;
                case kVK_ANSI_4: return KeyCode::num4;
                case kVK_ANSI_5: return KeyCode::num5;
                case kVK_ANSI_6: return KeyCode::num6;
                case kVK_ANSI_7: return KeyCode::num7;
                case kVK_ANSI_8: return KeyCode::num8;
                case kVK_ANSI_9: return KeyCode::num9;
                case kVK_ANSI_Equal: return KeyCode::equal;
                case kVK_ANSI_Minus: return KeyCode::minus;
                case kVK_Delete: return KeyCode::backspace;
                case kVK_ANSI_A: return KeyCode::a;
                case kVK_ANSI_B: return KeyCode::b;
                case kVK_ANSI_C: return KeyCode::c;
                case kVK_ANSI_D: return KeyCode::d;
                case kVK_ANSI_E: return KeyCode::e;
                case kVK_ANSI_F: return KeyCode::f;
                case kVK_ANSI_G: return KeyCode::g;
                case kVK_ANSI_H: return KeyCode::h;
                case kVK_ANSI_I: return KeyCode::i;
                case kVK_ANSI_J: return KeyCode::j;
                case kVK_ANSI_K: return KeyCode::k;
                case kVK_ANSI_L: return KeyCode::l;
                case kVK_ANSI_M: return KeyCode::m;
                case kVK_ANSI_N: return KeyCode::n;
                case kVK_ANSI_O: return KeyCode::o;
                case kVK_ANSI_P: return KeyCode::p;
                case kVK_ANSI_Q: return KeyCode::q;
                case kVK_ANSI_R: return KeyCode::r;
                case kVK_ANSI_S: return KeyCode::s;
                case kVK_ANSI_T: return KeyCode::t;
                case kVK_ANSI_U: return KeyCode::u;
                case kVK_ANSI_V: return KeyCode::v;
                case kVK_ANSI_W: return KeyCode::w;
                case kVK_ANSI_X: return KeyCode::x;
                case kVK_ANSI_Y: return KeyCode::y;
                case kVK_ANSI_Z: return KeyCode::z;
                case kVK_Tab: return KeyCode::tab;
                case kVK_CapsLock: return KeyCode::caps_lock;
                case kVK_Return: return KeyCode::enter;
                case kVK_Control: return KeyCode::l_ctrl;
                case kVK_RightControl: return KeyCode::r_ctrl;
                case kVK_Shift: return KeyCode::l_shift;
                case kVK_RightShift: return KeyCode::r_shift;
                case kVK_Option: return KeyCode::l_menu;
                case kVK_RightOption: return KeyCode::r_menu;
                case kVK_Command: return KeyCode::l_system;
                case kVK_RightCommand: return KeyCode::r_system;
                case kVK_Space: return KeyCode::spacebar;
                case kVK_ANSI_LeftBracket: return KeyCode::l_branket;
                case kVK_ANSI_RightBracket: return KeyCode::r_branket;
                case kVK_ANSI_Backslash: return KeyCode::backslash;
                case kVK_ANSI_Semicolon: return KeyCode::semicolon;
                case kVK_ANSI_Quote: return KeyCode::quote;
                case kVK_ANSI_Comma: return KeyCode::comma;
                case kVK_ANSI_Period: return KeyCode::period;
                case kVK_ANSI_Slash: return KeyCode::slash;
                case kVK_F13: return KeyCode::print_screen;
                case kVK_F14: return KeyCode::scroll_lock;
                case kVK_F15: return KeyCode::pause;
                case kVK_Help: return KeyCode::insert;
                case kVK_Home: return KeyCode::home;
                case kVK_PageUp: return KeyCode::page_up;
                case kVK_PageDown: return KeyCode::page_down;
                case kVK_ForwardDelete: return KeyCode::del;
                case kVK_End: return KeyCode::end;
                case kVK_LeftArrow: return KeyCode::left;
                case kVK_UpArrow: return KeyCode::up;
                case kVK_RightArrow: return KeyCode::right;
                case kVK_DownArrow: return KeyCode::down;
                case kVK_ANSI_KeypadClear: return KeyCode::num_lock;
                case kVK_ANSI_Keypad0: return KeyCode::numpad0;
                case kVK_ANSI_Keypad1: return KeyCode::numpad1;
                case kVK_ANSI_Keypad2: return KeyCode::numpad2;
                case kVK_ANSI_Keypad3: return KeyCode::numpad3;
                case kVK_ANSI_Keypad4: return KeyCode::numpad4;
                case kVK_ANSI_Keypad5: return KeyCode::numpad5;
                case kVK_ANSI_Keypad6: return KeyCode::numpad6;
                case kVK_ANSI_Keypad7: return KeyCode::numpad7;
                case kVK_ANSI_Keypad8: return KeyCode::numpad8;
                case kVK_ANSI_Keypad9: return KeyCode::numpad9;
                case kVK_ANSI_KeypadDecimal: return KeyCode::numpad_decimal;
                case kVK_ANSI_KeypadPlus: return KeyCode::numpad_add;
                case kVK_ANSI_KeypadMinus: return KeyCode::numpad_subtract;
                case kVK_ANSI_KeypadMultiply: return KeyCode::numpad_multiply;
                case kVK_ANSI_KeypadDivide: return KeyCode::numpad_divide;
                case kVK_ANSI_KeypadEquals: return KeyCode::numpad_equal;
                case kVK_ANSI_KeypadEnter: return KeyCode::numpad_enter;
                default: break;
            }
            return KeyCode::unknown;
        }

        inline HID::MouseButton translate_mouse_button(NSInteger buttonNumber)
        {
            using namespace HID;
            switch(buttonNumber)
            {
                case 0: return MouseButton::left;
                case 1: return MouseButton::right;
                case 2: return MouseButton::middle;
                case 3: return MouseButton::function1;
                case 4: return MouseButton::function2;
                default: return MouseButton::none;
            }
        }

        static void process_cocoa_event(NSEvent* event);

        LUNA_WINDOW_API void poll_cocoa_events()
        {
            lutsassert_main_thread();
            @autoreleasepool
            {
                NSEvent* event = nil;
                while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                   untilDate:nil
                                                      inMode:NSDefaultRunLoopMode
                                                     dequeue:YES]))
                {
                    process_cocoa_event(event);
                }
            }
        }

        LUNA_WINDOW_API void handle_cocoa_event(void* nsevent_ptr)
        {
            NSEvent* event = (__bridge NSEvent*)nsevent_ptr;
            process_cocoa_event(event);
        }

        static void process_cocoa_event(NSEvent* event)
        {
            @autoreleasepool
            {
                NSWindow* nswindow = [event window];
                if(!nswindow)
                {
                    [NSApp sendEvent:event];
                    return;
                }
                NSValue* value = objc_getAssociatedObject(nswindow, (const void*)WINDOW_POINTER_KEY);
                Window* window = nullptr;
                if(value)
                {
                    window = (Window*)[value pointerValue];
                }
                if(window && !window->is_closed())
                {
                    switch ([event type])
                    {
                        case NSEventTypeKeyDown:
                        {
                            HID::KeyCode key = translate_key([event keyCode]);
                            if (key != HID::KeyCode::unknown)
                            {
                                window->get_events().key_down(window, key);
                            }
                            
                            // Handle text input
                            if (window->m_text_input_active)
                            {
                                NSString* characters = [event characters];
                                if (characters && [characters length] > 0)
                                {
                                    const char* utf8 = [characters UTF8String];
                                    if (utf8)
                                    {
                                        const char* cur = utf8;
                                        while(*cur)
                                        {
                                            c32 ch = utf8_decode_char(cur);
                                            cur += utf8_charspan(ch);
                                            window->get_events().input_character(window, ch);
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        
                        case NSEventTypeKeyUp:
                        {
                            HID::KeyCode key = translate_key([event keyCode]);
                            if (key != HID::KeyCode::unknown)
                            {
                                window->get_events().key_up(window, key);
                            }
                            break;
                        }
                        
                        case NSEventTypeMouseMoved:
                        case NSEventTypeLeftMouseDragged:
                        case NSEventTypeRightMouseDragged:
                        case NSEventTypeOtherMouseDragged:
                        {
                            NSPoint locationInWindow = [event locationInWindow];
                            NSView* contentView = [nswindow contentView];
                            
                            // Convert to client coordinates (flip Y axis)
                            CGFloat height = [contentView bounds].size.height;
                            i32 x = (i32)locationInWindow.x;
                            i32 y = (i32)(height - locationInWindow.y);
                            
                            window->get_events().mouse_move(window, x, y);
                            break;
                        }
                        
                        case NSEventTypeLeftMouseDown:
                        case NSEventTypeRightMouseDown:
                        case NSEventTypeOtherMouseDown:
                        {
                            HID::MouseButton button = translate_mouse_button([event buttonNumber]);
                            if (button != HID::MouseButton::none)
                            {
                                window->get_events().mouse_down(window, button);
                            }
                            break;
                        }
                        
                        case NSEventTypeLeftMouseUp:
                        case NSEventTypeRightMouseUp:
                        case NSEventTypeOtherMouseUp:
                        {
                            HID::MouseButton button = translate_mouse_button([event buttonNumber]);
                            if (button != HID::MouseButton::none)
                            {
                                window->get_events().mouse_up(window, button);
                            }
                            break;
                        }
                        
                        case NSEventTypeScrollWheel:
                        {
                            CGFloat dx = [event scrollingDeltaX];
                            CGFloat dy = [event scrollingDeltaY];
                            
                            // macOS uses "natural" scrolling, so we may need to invert
                            if ([event hasPreciseScrollingDeltas])
                            {
                                // Trackpad
                                dx *= 0.1f;
                                dy *= 0.1f;
                            }
                            
                            window->get_events().scroll(window, (f32)dx, (f32)dy);
                            break;
                        }
                        
                        case NSEventTypeMouseEntered:
                        {
                            window->get_events().mouse_enter(window);
                            break;
                        }
                        
                        case NSEventTypeMouseExited:
                        {
                            window->get_events().mouse_leave(window);
                            break;
                        }
                        
                        default:
                            break;
                    }
                }
                // Always dispatch the event to the system
                [NSApp sendEvent:event];
            }
        }
    }
}
