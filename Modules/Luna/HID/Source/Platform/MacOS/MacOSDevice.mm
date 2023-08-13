/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MacOSDevice.mm
* @author JXMaster
* @date 2023/8/13
*/
#include "../../HID.hpp"
#include "../../../HID.hpp"
#include "../../../Mouse.hpp"
#include "../../../Keyboard.hpp"
#include "../../../Controller.hpp"
#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

namespace Luna
{
    namespace HID
    {
        struct MacOSDevice : IMouse, IKeyboard
        {
            lustruct("HID::MacOSDevice", "{7f51cc43-2d1c-40ce-a3e6-f15f9a76b31b}");
            luiimpl();

            virtual Int2U get_cursor_pos() override;
			virtual bool get_button_state(MouseButton mouse_button) override;
			virtual RV set_cursor_pos(i32 x, i32 y) override;
			virtual bool get_key_state(KeyCode key) override;
        };

        Ref<MacOSDevice> g_device;

        Int2U MacOSDevice::get_cursor_pos()
        {
            @autoreleasepool
            {
                CGEventRef mouse_event = CGEventCreate(NULL);
                CGPoint mouse_position = CGEventGetLocation(mouse_event);
                CFRelease(mouse_event);
                return Int2U(mouse_position.x, mouse_position.y);
            }
        }
        bool MacOSDevice::get_button_state(MouseButton mouse_button)
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
        RV MacOSDevice::set_cursor_pos(i32 x, i32 y)
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
        static CGKeyCode map_key_code(KeyCode key)
        {
            switch(key)
            {
                case KeyCode::esc: return kVK_Escape;
                case KeyCode::f1: return kVK_F1;
                case KeyCode::f2: return kVK_F2;
                case KeyCode::f3: return kVK_F3;
                case KeyCode::f4: return kVK_F4;
                case KeyCode::f5: return kVK_F5;
                case KeyCode::f6: return kVK_F6;
                case KeyCode::f7: return kVK_F7;
                case KeyCode::f8: return kVK_F8;
                case KeyCode::f9: return kVK_F9;
                case KeyCode::f10: return kVK_F10;
                case KeyCode::f11: return kVK_F11;
                case KeyCode::f12: return kVK_F12;
                case KeyCode::grave: return kVK_ANSI_Grave;
                case KeyCode::num0: return kVK_ANSI_0;
                case KeyCode::num1: return kVK_ANSI_1;
                case KeyCode::num2: return kVK_ANSI_2;
                case KeyCode::num3: return kVK_ANSI_3;
                case KeyCode::num4: return kVK_ANSI_4;
                case KeyCode::num5: return kVK_ANSI_5;
                case KeyCode::num6: return kVK_ANSI_6;
                case KeyCode::num7: return kVK_ANSI_7;
                case KeyCode::num8: return kVK_ANSI_8;
                case KeyCode::num9: return kVK_ANSI_9;
                case KeyCode::equal: return kVK_ANSI_Equal;
                case KeyCode::minus: return kVK_ANSI_Minus;
                case KeyCode::backspace: return kVK_Delete;
                case KeyCode::a: return kVK_ANSI_A;
                case KeyCode::b: return kVK_ANSI_B;
                case KeyCode::c: return kVK_ANSI_C;
                case KeyCode::d: return kVK_ANSI_D;
                case KeyCode::e: return kVK_ANSI_E;
                case KeyCode::f: return kVK_ANSI_F;
                case KeyCode::g: return kVK_ANSI_G;
                case KeyCode::h: return kVK_ANSI_H;
                case KeyCode::i: return kVK_ANSI_I;
                case KeyCode::j: return kVK_ANSI_J;
                case KeyCode::k: return kVK_ANSI_K;
                case KeyCode::l: return kVK_ANSI_L;
                case KeyCode::m: return kVK_ANSI_M;
                case KeyCode::n: return kVK_ANSI_N;
                case KeyCode::o: return kVK_ANSI_O;
                case KeyCode::p: return kVK_ANSI_P;
                case KeyCode::q: return kVK_ANSI_Q;
                case KeyCode::r: return kVK_ANSI_R;
                case KeyCode::s: return kVK_ANSI_S;
                case KeyCode::t: return kVK_ANSI_T;
                case KeyCode::u: return kVK_ANSI_U;
                case KeyCode::v: return kVK_ANSI_V;
                case KeyCode::w: return kVK_ANSI_W;
                case KeyCode::x: return kVK_ANSI_X;
                case KeyCode::y: return kVK_ANSI_Y;
                case KeyCode::z: return kVK_ANSI_Z;
                case KeyCode::tab: return kVK_Tab;
                case KeyCode::caps_lock: return kVK_CapsLock;
                case KeyCode::enter: return kVK_Return;
                case KeyCode::ctrl: return kVK_Control;
                case KeyCode::l_ctrl: return kVK_Control;
                case KeyCode::r_ctrl: return kVK_RightControl;
                case KeyCode::shift: return kVK_Shift;
                case KeyCode::l_shift: return kVK_Shift;
                case KeyCode::r_shift: return kVK_RightShift;
                case KeyCode::menu: return kVK_Option;
                case KeyCode::l_menu: return kVK_Option;
                case KeyCode::r_menu: return kVK_RightOption;
                case KeyCode::l_system: return kVK_Command;
                case KeyCode::r_system: return kVK_RightCommand;
                case KeyCode::spacebar: return kVK_Space;
                case KeyCode::l_branket: return kVK_ANSI_LeftBracket;
                case KeyCode::r_branket: return kVK_ANSI_RightBracket;
                case KeyCode::backslash: return kVK_ANSI_Backslash;
                case KeyCode::semicolon: return kVK_ANSI_Semicolon;
                case KeyCode::quote: return kVK_ANSI_Quote;
                case KeyCode::comma: return kVK_ANSI_Comma;
                case KeyCode::period: return kVK_ANSI_Period;
                case KeyCode::slash: return kVK_ANSI_Slash;
                case KeyCode::print_screen: return kVK_F13;
                case KeyCode::scroll_lock: return kVK_F14;
                case KeyCode::pause: return kVK_F15;
                case KeyCode::insert: return kVK_Help;
                case KeyCode::home: return kVK_Home;
                case KeyCode::page_up: return kVK_PageUp;
                case KeyCode::page_down: return kVK_PageDown;
                case KeyCode::del: return kVK_ForwardDelete;
                case KeyCode::end: return kVK_End;
                case KeyCode::left: return kVK_LeftArrow;
                case KeyCode::up: return kVK_UpArrow;
                case KeyCode::right: return kVK_RightArrow;
                case KeyCode::down: return kVK_DownArrow;
                case KeyCode::num_lock: return kVK_ANSI_KeypadClear;
                case KeyCode::numpad0: return kVK_ANSI_Keypad0;
                case KeyCode::numpad1: return kVK_ANSI_Keypad1;
                case KeyCode::numpad2: return kVK_ANSI_Keypad2;
                case KeyCode::numpad3: return kVK_ANSI_Keypad3;
                case KeyCode::numpad4: return kVK_ANSI_Keypad4;
                case KeyCode::numpad5: return kVK_ANSI_Keypad5;
                case KeyCode::numpad6: return kVK_ANSI_Keypad6;
                case KeyCode::numpad7: return kVK_ANSI_Keypad7;
                case KeyCode::numpad8: return kVK_ANSI_Keypad8;
                case KeyCode::numpad9: return kVK_ANSI_Keypad9;
                case KeyCode::numpad_decimal: return kVK_ANSI_KeypadDecimal;
                case KeyCode::numpad_add: return kVK_ANSI_KeypadPlus;
                case KeyCode::numpad_subtract: return kVK_ANSI_KeypadMinus;
                case KeyCode::numpad_multiply: return kVK_ANSI_KeypadMultiply;
                case KeyCode::numpad_divide: return kVK_ANSI_KeypadDivide;
                case KeyCode::numpad_equal: return kVK_ANSI_KeypadEquals;
                case KeyCode::numpad_enter: return
                    kVK_ANSI_KeypadEnter;
                default: break;
            }
            return 0;
        }
        bool MacOSDevice::get_key_state(KeyCode key)
        {
            @autoreleasepool
            {
                CGKeyCode code = map_key_code(key);
                if(code == 0) return false;
                bool pressed = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, code);
                return pressed;
            }
        }
        RV request_macos_device(void* userdata, const Guid& iid, object_t* out_device_object)
		{
			object_t obj = g_device.object();
			object_retain(obj);
			*out_device_object = obj;
			return ok;
		}
		RV register_platform_devices()
		{
			register_boxed_type<MacOSDevice>();
			impl_interface_for_type<MacOSDevice, IMouse, IKeyboard>();
			g_device = new_object<MacOSDevice>();
			DeviceDesc desc;
			desc.userdata = nullptr;
			desc.supported_iids = { IMouse::__guid, IKeyboard::__guid};
			desc.on_request_device = request_macos_device;
			desc.on_unregister = nullptr;
			return register_device("MacOSDevice", desc);
		}
		void unregister_platform_devices()
		{
			unregister_device("MacOSDevice");
			g_device.reset();
		}
    }
}
