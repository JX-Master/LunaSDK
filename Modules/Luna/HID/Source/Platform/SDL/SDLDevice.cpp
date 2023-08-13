/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SDLDevice.cpp
* @author JXMaster
* @date 2022/4/1
*/
#ifdef LUNA_HID_SDL
#include "../../HID.hpp"
#include "../../../HID.hpp"
#include "../../../Mouse.hpp"
#include "../../../Keyboard.hpp"
#include "../../../Controller.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_gamecontroller.h>

namespace Luna
{
    namespace HID
    {
        struct SDLDevice : IMouse, IKeyboard
        {
            lustruct("HID::SDLDevice", "{321ab214-a131-4333-8ae0-6a1bb80dcfbb}");
            luiimpl();

            virtual Int2U get_cursor_pos() override;
			virtual bool get_button_state(MouseButton mouse_button) override;
			virtual RV set_cursor_pos(i32 x, i32 y) override;
			virtual bool get_key_state(KeyCode key) override;
			//virtual ControllerInputState get_state(u32 index) override;
			//virtual RV set_state(u32 index, const ControllerOutputState& state) override;
        };

        Ref<SDLDevice> g_device;

        Int2U SDLDevice::get_cursor_pos()
        {
            int x, y;
            SDL_GetMouseState(&x, &y);
            return Int2U(x, y);
        }

        bool SDLDevice::get_button_state(MouseButton mouse_button)
        {
            Uint32 buttons = SDL_GetMouseState(nullptr, nullptr);
            switch (mouse_button)
            {
            case MouseButton::left:
                return (buttons & SDL_BUTTON_LMASK) != 0;
            case MouseButton::right:
                return (buttons & SDL_BUTTON_RMASK) != 0;
            case MouseButton::middle:
                return (buttons & SDL_BUTTON_MMASK) != 0;
            case MouseButton::function1:
                return (buttons & SDL_BUTTON_X1MASK) != 0;
            case MouseButton::function2:
                return (buttons & SDL_BUTTON_X2MASK) != 0;
            default: break;
            }
            return false;
        }

        RV SDLDevice::set_cursor_pos(i32 x, i32 y)
        {
            int r = SDL_WarpMouseGlobal(x, y);
            if (r)
            {
                return set_error(BasicError::bad_platform_call(), "%s", SDL_GetError());
            }
            return ok;
        }

        static SDL_Scancode map_scan_code(KeyCode key)
        {
            if ((u16)key >= (u16)KeyCode::num0 && (u16)key <= (u16)KeyCode::num9)
            {
                return (SDL_Scancode)((u16)key - (u16)KeyCode::num0 + SDL_SCANCODE_0);
            }
            if ((u16)key >= (u16)KeyCode::a && (u16)key <= (u16)KeyCode::z)
            {
                return (SDL_Scancode)((u16)key - (u16)KeyCode::a + SDL_SCANCODE_A);
            }
            if ((u16)key >= (u16)KeyCode::f1 && (u16)key <= (u16)KeyCode::f12)
            {
                return (SDL_Scancode)((u16)key - (u16)KeyCode::f1 + SDL_SCANCODE_F1);
            }
            if ((u16)key >= (u16)KeyCode::numpad1 && (u16)key <= (u16)KeyCode::numpad9)
            {
                return (SDL_Scancode)((u16)key - (u16)KeyCode::numpad1 + SDL_SCANCODE_KP_1);
            }
            switch (key)
            {
            case KeyCode::esc:				return SDL_SCANCODE_ESCAPE;
            case KeyCode::grave:			return SDL_SCANCODE_GRAVE;
            case KeyCode::equal:			return SDL_SCANCODE_EQUALS;
            case KeyCode::minus:			return SDL_SCANCODE_MINUS;
            case KeyCode::backspace:		return SDL_SCANCODE_BACKSPACE;
            case KeyCode::tab:				return SDL_SCANCODE_TAB;
            case KeyCode::caps_lock:		return SDL_SCANCODE_CAPSLOCK;
            case KeyCode::enter:			return SDL_SCANCODE_RETURN;
            case KeyCode::l_ctrl:			return SDL_SCANCODE_LCTRL;
            case KeyCode::r_ctrl:			return SDL_SCANCODE_RCTRL;
            case KeyCode::l_shift:			return SDL_SCANCODE_LSHIFT;
            case KeyCode::r_shift:			return SDL_SCANCODE_RSHIFT;
            case KeyCode::menu:				return SDL_SCANCODE_MENU;
            case KeyCode::l_menu:			return SDL_SCANCODE_LALT;
            case KeyCode::r_menu:			return SDL_SCANCODE_RALT;
            case KeyCode::l_system:			return SDL_SCANCODE_LGUI;
            case KeyCode::r_system:			return SDL_SCANCODE_RGUI;
            case KeyCode::apps:				return SDL_SCANCODE_APPLICATION;
            case KeyCode::spacebar:			return SDL_SCANCODE_SPACE;
            case KeyCode::l_branket:		return SDL_SCANCODE_LEFTBRACKET;
            case KeyCode::r_branket:		return SDL_SCANCODE_RIGHTBRACKET;
            case KeyCode::backslash:		return SDL_SCANCODE_BACKSLASH;
            case KeyCode::semicolon:		return SDL_SCANCODE_SEMICOLON;
            case KeyCode::quote:			return SDL_SCANCODE_APOSTROPHE;
            case KeyCode::comma:			return SDL_SCANCODE_COMMA;
            case KeyCode::period:			return SDL_SCANCODE_PERIOD;
            case KeyCode::slash:			return SDL_SCANCODE_SLASH;
            case KeyCode::print_screen:		return SDL_SCANCODE_PRINTSCREEN;
            case KeyCode::scroll_lock:		return SDL_SCANCODE_SCROLLLOCK;
            case KeyCode::pause:			return SDL_SCANCODE_PAUSE;
            case KeyCode::insert:			return SDL_SCANCODE_INSERT;
            case KeyCode::home:				return SDL_SCANCODE_HOME;
            case KeyCode::page_up:			return SDL_SCANCODE_PAGEUP;
            case KeyCode::page_down:		return SDL_SCANCODE_PAGEDOWN;
            case KeyCode::del:				return SDL_SCANCODE_DELETE;
            case KeyCode::end:				return SDL_SCANCODE_END;
            case KeyCode::left:				return SDL_SCANCODE_LEFT;
            case KeyCode::up:				return SDL_SCANCODE_UP;
            case KeyCode::right:			return SDL_SCANCODE_RIGHT;
            case KeyCode::down:				return SDL_SCANCODE_DOWN;
            case KeyCode::num_lock:			return SDL_SCANCODE_NUMLOCKCLEAR;
            case KeyCode::numpad_decimal:	return SDL_SCANCODE_KP_PERIOD;
            case KeyCode::numpad_add:		return SDL_SCANCODE_KP_PLUS;
            case KeyCode::numpad_subtract:	return SDL_SCANCODE_KP_MINUS;
            case KeyCode::numpad_multiply:	return SDL_SCANCODE_KP_MULTIPLY;
            case KeyCode::numpad_divide:	return SDL_SCANCODE_KP_DIVIDE;
            case KeyCode::clear:			return SDL_SCANCODE_CLEAR;
            case KeyCode::numpad0:          return SDL_SCANCODE_KP_0;
            default: lupanic(); return SDL_SCANCODE_UNKNOWN;
            }
        }

        bool SDLDevice::get_key_state(KeyCode key)
        {
            SDL_PumpEvents();
            SDL_Scancode code = map_scan_code(key);
            int numkeys;
            auto states = SDL_GetKeyboardState(&numkeys);
            if (code >= numkeys) return false;
            if (key == KeyCode::ctrl)
            {
                return states[SDL_SCANCODE_LCTRL] == 1 || states[SDL_SCANCODE_RCTRL] == 1;
            }
            else if (key == KeyCode::shift)
            {
                return states[SDL_SCANCODE_LSHIFT] == 1 || states[SDL_SCANCODE_RSHIFT] == 1;
            }
            return states[code] == 1;
        }
    
        RV request_sdl_device(void* userdata, const Guid& iid, object_t* out_device_object)
        {
            object_t obj = g_device.object();
            object_retain(obj);
            *out_device_object = obj;
            return ok;
        }
    
        RV register_platform_devices()
        {
            register_boxed_type<SDLDevice>();
            impl_interface_for_type<SDLDevice, IMouse, IKeyboard>();
            g_device = new_object<SDLDevice>();
            DeviceDesc desc;
            desc.userdata = nullptr;
            desc.supported_iids = { IMouse::__guid, IKeyboard::__guid};
            desc.on_request_device = request_sdl_device;
            desc.on_unregister = nullptr;
            return register_device("SDLDevice", desc);
        }

        void unregister_platform_devices()
        {
            unregister_device("SDLDevice");
            g_device.reset();
        }

    }
}

#endif
