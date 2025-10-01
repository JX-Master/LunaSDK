/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventDispatching.cpp
* @author JXMaster
* @date 2025/3/24
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../SDL/EventHandling.hpp"
#include "../../EventDispatching.hpp"
#include "Window.hpp"
#include "Display.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Thread.hpp>

namespace Luna
{
    namespace Window
    {
        inline Window* get_window_from_sdl_window_id(SDL_WindowID window_id)
        {
            SDL_Window* sdl_window = SDL_GetWindowFromID(window_id);
            if(!sdl_window) return nullptr;
            return (Window*)SDL_GetPointerProperty(SDL_GetWindowProperties(sdl_window), "LunaWindow", nullptr);
        }

        inline HID::KeyCode translate_key(SDL_Scancode key)
        {
            using namespace HID;
            switch(key)
            {
                case SDL_SCANCODE_A: return KeyCode::a;
                case SDL_SCANCODE_B: return KeyCode::b;
                case SDL_SCANCODE_C: return KeyCode::c;
                case SDL_SCANCODE_D: return KeyCode::d;
                case SDL_SCANCODE_E: return KeyCode::e;
                case SDL_SCANCODE_F: return KeyCode::f;
                case SDL_SCANCODE_G: return KeyCode::g;
                case SDL_SCANCODE_H: return KeyCode::h;
                case SDL_SCANCODE_I: return KeyCode::i;
                case SDL_SCANCODE_J: return KeyCode::j;
                case SDL_SCANCODE_K: return KeyCode::k;
                case SDL_SCANCODE_L: return KeyCode::l;
                case SDL_SCANCODE_M: return KeyCode::m;
                case SDL_SCANCODE_N: return KeyCode::n;
                case SDL_SCANCODE_O: return KeyCode::o;
                case SDL_SCANCODE_P: return KeyCode::p;
                case SDL_SCANCODE_Q: return KeyCode::q;
                case SDL_SCANCODE_R: return KeyCode::r;
                case SDL_SCANCODE_S: return KeyCode::s;
                case SDL_SCANCODE_T: return KeyCode::t;
                case SDL_SCANCODE_U: return KeyCode::u;
                case SDL_SCANCODE_V: return KeyCode::v;
                case SDL_SCANCODE_W: return KeyCode::w;
                case SDL_SCANCODE_X: return KeyCode::x;
                case SDL_SCANCODE_Y: return KeyCode::y;
                case SDL_SCANCODE_Z: return KeyCode::z;
                case SDL_SCANCODE_1: return KeyCode::num1;
                case SDL_SCANCODE_2: return KeyCode::num2;
                case SDL_SCANCODE_3: return KeyCode::num3;
                case SDL_SCANCODE_4: return KeyCode::num4;
                case SDL_SCANCODE_5: return KeyCode::num5;
                case SDL_SCANCODE_6: return KeyCode::num6;
                case SDL_SCANCODE_7: return KeyCode::num7;
                case SDL_SCANCODE_8: return KeyCode::num8;
                case SDL_SCANCODE_9: return KeyCode::num9;
                case SDL_SCANCODE_0: return KeyCode::num0;
                case SDL_SCANCODE_RETURN: return KeyCode::enter;
                case SDL_SCANCODE_ESCAPE: return KeyCode::esc;
                case SDL_SCANCODE_BACKSPACE: return KeyCode::backspace;
                case SDL_SCANCODE_TAB: return KeyCode::tab;
                case SDL_SCANCODE_SPACE: return KeyCode::spacebar;
                case SDL_SCANCODE_MINUS: return KeyCode::minus;
                case SDL_SCANCODE_EQUALS: return KeyCode::equal;
                case SDL_SCANCODE_LEFTBRACKET: return KeyCode::l_branket;
                case SDL_SCANCODE_RIGHTBRACKET: return KeyCode::r_branket;
                case SDL_SCANCODE_BACKSLASH: 
                case SDL_SCANCODE_NONUSHASH: return KeyCode::backslash;
                case SDL_SCANCODE_SEMICOLON: return KeyCode::semicolon;
                case SDL_SCANCODE_APOSTROPHE: return KeyCode::quote;
                case SDL_SCANCODE_GRAVE: return KeyCode::grave;
                case SDL_SCANCODE_COMMA: return KeyCode::comma;
                case SDL_SCANCODE_PERIOD: return KeyCode::period;
                case SDL_SCANCODE_SLASH: return KeyCode::slash;
                case SDL_SCANCODE_CAPSLOCK: return KeyCode::caps_lock;
                case SDL_SCANCODE_F1: return KeyCode::f1;
                case SDL_SCANCODE_F2: return KeyCode::f2;
                case SDL_SCANCODE_F3: return KeyCode::f3;
                case SDL_SCANCODE_F4: return KeyCode::f4;
                case SDL_SCANCODE_F5: return KeyCode::f5;
                case SDL_SCANCODE_F6: return KeyCode::f6;
                case SDL_SCANCODE_F7: return KeyCode::f7;
                case SDL_SCANCODE_F8: return KeyCode::f8;
                case SDL_SCANCODE_F9: return KeyCode::f9;
                case SDL_SCANCODE_F10: return KeyCode::f10;
                case SDL_SCANCODE_F11: return KeyCode::f11;
                case SDL_SCANCODE_F12: return KeyCode::f12;
                case SDL_SCANCODE_PRINTSCREEN: return KeyCode::print_screen;
                case SDL_SCANCODE_SCROLLLOCK: return KeyCode::scroll_lock;
                case SDL_SCANCODE_PAUSE: return KeyCode::pause;
                case SDL_SCANCODE_INSERT: return KeyCode::insert;
                case SDL_SCANCODE_HOME: return KeyCode::home;
                case SDL_SCANCODE_PAGEUP: return KeyCode::page_up;
                case SDL_SCANCODE_DELETE: return KeyCode::del;
                case SDL_SCANCODE_END: return KeyCode::end;
                case SDL_SCANCODE_PAGEDOWN: return KeyCode::page_down;
                case SDL_SCANCODE_RIGHT: return KeyCode::right;
                case SDL_SCANCODE_LEFT: return KeyCode::left;
                case SDL_SCANCODE_DOWN: return KeyCode::down;
                case SDL_SCANCODE_UP: return KeyCode::up;
                case SDL_SCANCODE_NUMLOCKCLEAR: return KeyCode::num_lock;
                case SDL_SCANCODE_KP_DIVIDE: return KeyCode::numpad_divide;
                case SDL_SCANCODE_KP_MULTIPLY: return KeyCode::numpad_multiply;
                case SDL_SCANCODE_KP_MINUS: return KeyCode::numpad_subtract;
                case SDL_SCANCODE_KP_PLUS: return KeyCode::numpad_add;
                case SDL_SCANCODE_KP_ENTER: return KeyCode::numpad_enter;
                case SDL_SCANCODE_KP_1: return KeyCode::numpad1;
                case SDL_SCANCODE_KP_2: return KeyCode::numpad2;
                case SDL_SCANCODE_KP_3: return KeyCode::numpad3;
                case SDL_SCANCODE_KP_4: return KeyCode::numpad4;
                case SDL_SCANCODE_KP_5: return KeyCode::numpad5;
                case SDL_SCANCODE_KP_6: return KeyCode::numpad6;
                case SDL_SCANCODE_KP_7: return KeyCode::numpad7;
                case SDL_SCANCODE_KP_8: return KeyCode::numpad8;
                case SDL_SCANCODE_KP_9: return KeyCode::numpad9;
                case SDL_SCANCODE_KP_0: return KeyCode::numpad0;
                case SDL_SCANCODE_KP_PERIOD: return KeyCode::numpad_decimal;
                case SDL_SCANCODE_NONUSBACKSLASH: return KeyCode::backslash;
                case SDL_SCANCODE_APPLICATION: return KeyCode::apps;
                case SDL_SCANCODE_KP_EQUALS: return KeyCode::numpad_equal;
                case SDL_SCANCODE_LCTRL: return KeyCode::l_ctrl;
                case SDL_SCANCODE_LSHIFT: return KeyCode::l_shift;
                case SDL_SCANCODE_LALT: return KeyCode::l_menu;
                case SDL_SCANCODE_LGUI: return KeyCode::l_system;
                case SDL_SCANCODE_RCTRL: return KeyCode::r_ctrl;
                case SDL_SCANCODE_RSHIFT: return KeyCode::r_shift;
                case SDL_SCANCODE_RALT: return KeyCode::r_menu;
                case SDL_SCANCODE_RGUI: return KeyCode::r_system;
                default: break;
            }
            return KeyCode::unknown;
        }

        LUNA_WINDOW_API void handle_sdl_event(SDL_Event& event)
        {
            if(event.type >= SDL_EVENT_DISPLAY_FIRST && event.type <= SDL_EVENT_DISPLAY_LAST)
            {
                Display* display = (Display*)get_display_from_display_id(event.display.displayID);
                switch(event.type)
                {
                    case SDL_EVENT_DISPLAY_ORIENTATION:
                    DisplayOrientation orientation;
                    switch(event.display.data1)
                    {
                        case SDL_ORIENTATION_UNKNOWN:
                        orientation = DisplayOrientation::unknown;
                        break;
                        case SDL_ORIENTATION_LANDSCAPE:
                        orientation = DisplayOrientation::landscape;
                        break;
                        case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                        orientation = DisplayOrientation::landscape_flipped;
                        break;
                        case SDL_ORIENTATION_PORTRAIT:
                        orientation = DisplayOrientation::portrait;
                        break;
                        case SDL_ORIENTATION_PORTRAIT_FLIPPED:
                        orientation = DisplayOrientation::portrait_flipped;
                        break;
                    }
                    dispatch_display_orientation_event(display, orientation);
                    break;
                    case SDL_EVENT_DISPLAY_ADDED:
                    dispatch_display_connect_event(display);
                    break;
                    case SDL_EVENT_DISPLAY_REMOVED:
                    dispatch_display_disconnect_event(display);
                    break;
                    case SDL_EVENT_DISPLAY_MOVED:
                    dispatch_display_move_event(display);
                    break;
                    default:
                    lupanic();
                    break;
                }
            }
            else if(event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
            {
                Window* window = get_window_from_sdl_window_id(event.window.windowID);
                if(window)
                {
                    switch(event.type)
                    {
                        case SDL_EVENT_WINDOW_SHOWN:
                        dispatch_window_show_event(window);
                        break;
                        case SDL_EVENT_WINDOW_HIDDEN:
                        dispatch_window_hide_event(window);
                        break;
                        case SDL_EVENT_WINDOW_MOVED:
                        dispatch_window_move_event(window, event.window.data1, event.window.data2);
                        break;
                        case SDL_EVENT_WINDOW_RESIZED:
                        dispatch_window_resize_event(window, event.window.data1, event.window.data2);
                        break;
                        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                        dispatch_window_framebuffer_resize_event(window, event.window.data1, event.window.data2);
                        break;
                        case SDL_EVENT_WINDOW_MOUSE_ENTER:
                        dispatch_window_mouse_enter_event(window);
                        break;
                        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                        dispatch_window_mouse_leave_event(window);
                        break;
                        case SDL_EVENT_WINDOW_FOCUS_GAINED:
                        dispatch_window_focus_event(window);
                        break;
                        case SDL_EVENT_WINDOW_FOCUS_LOST:
                        dispatch_window_lose_focus_event(window);
                        break;
                        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        dispatch_window_close_event(window);
                        break;
                        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                        dispatch_window_dpi_changed_event(window);
                        break;
                    }
                }
            }
            else if(event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
            {
                Window* window = get_window_from_sdl_window_id(event.key.windowID);
                if(window)
                {
                    HID::KeyCode key = translate_key(event.key.scancode);
                    if (key != HID::KeyCode::unknown)
                    {
                        if (event.type == SDL_EVENT_KEY_DOWN)
                        {
                            dispatch_window_key_down_event(window, key);
                        }
                        else
                        {
                            dispatch_window_key_up_event(window, key);
                        }
                    }
                }
            }
            else if(event.type == SDL_EVENT_TEXT_INPUT)
            {
                Window* window = get_window_from_sdl_window_id(event.text.windowID);
                if(window)
                {
                    usize utf8_len = utf8_strlen(event.text.text);
                    const char* cur = event.text.text;
                    for(usize i = 0; i < utf8_len; ++i)
                    {
                        c32 ch = utf8_decode_char(cur);
                        cur += utf8_charspan(ch);
                        dispatch_window_input_character_event(window, ch);
                    }
                }
            }
            else if(event.type == SDL_EVENT_MOUSE_MOTION)
            {
                Window* window = get_window_from_sdl_window_id(event.motion.windowID);
                if(window)
                {
                    dispatch_window_mouse_move_event(window, event.motion.x, event.motion.y);
                }
            }
            else if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                Window* window = get_window_from_sdl_window_id(event.button.windowID);
                if(window)
                {
                    HID::MouseButton button = HID::MouseButton::none;
                    switch(event.button.button)
                    {
                        case SDL_BUTTON_LEFT: button = HID::MouseButton::left; break;
                        case SDL_BUTTON_MIDDLE: button = HID::MouseButton::middle; break;
                        case SDL_BUTTON_RIGHT: button = HID::MouseButton::right; break;
                        case SDL_BUTTON_X1: button = HID::MouseButton::function1; break;
                        case SDL_BUTTON_X2: button = HID::MouseButton::function2; break;
                        default: break;
                    }
                    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                    {
                        dispatch_window_mouse_down_event(window, button);
                    }
                    else
                    {
                        dispatch_window_mouse_up_event(window, button);
                    }
                }
            }
            else if(event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                Window* window = get_window_from_sdl_window_id(event.wheel.windowID);
                if(window)
                {
                    dispatch_window_scroll_event(window, event.wheel.x, event.wheel.y);
                }
            }
            else if(event.type == SDL_EVENT_FINGER_MOTION || event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_UP)
            {
                Window* window = get_window_from_sdl_window_id(event.tfinger.windowID);
                if(window)
                {
                    if(event.type == SDL_EVENT_FINGER_MOTION)
                    {
                        dispatch_window_touch_move_event(window, (u64)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    }
                    else if(event.type == SDL_EVENT_FINGER_DOWN)
                    {
                        dispatch_window_touch_down_event(window, (u64)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    }
                    else
                    {
                        dispatch_window_touch_up_event(window, (u64)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    }
                }
            }
            else if(event.type == SDL_EVENT_DROP_FILE || event.type == SDL_EVENT_DROP_BEGIN || event.type == SDL_EVENT_DROP_COMPLETE)
            {
                Window* window = get_window_from_sdl_window_id(event.drop.windowID);
                if(window)
                {
                    if(event.type == SDL_EVENT_DROP_FILE)
                    {
                        window->m_drop_files.push_back(event.drop.data);
                    }
                    else if(event.type == SDL_EVENT_DROP_BEGIN)
                    {
                        window->m_drop_files.clear();
                    }
                    else if(event.type == SDL_EVENT_DROP_COMPLETE)
                    {
                        Array<const c8*> files(window->m_drop_files.size());
                        for(usize i = 0; i < window->m_drop_files.size(); ++i)
                        {
                            files[i] = window->m_drop_files[i].c_str();
                        }
                        dispatch_window_drop_file_event(window, files.data(), files.size());
                        window->m_drop_files.clear();
                    }
                }
            }
        }

        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            lucheck_msg(get_current_thread() == get_main_thread(), "Window::poll_events must only be called from the main thread.");
            SDL_Event event;
            bool any_event;
            if(wait_events)
            {
                any_event = SDL_WaitEvent(&event);
            }
            else
            {
                any_event = SDL_PollEvent(&event);
            }
            while (any_event)
            {
                // Handle event.
                handle_sdl_event(event);
                any_event = SDL_PollEvent(&event);
            }
        }
    }
}