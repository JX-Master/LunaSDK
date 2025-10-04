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
#include "../../../SDL/EventHandling.hpp"
#include "Window.hpp"
#include "Display.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/TSAssert.hpp>

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
                    g_display_events.orientation(display, orientation);
                    break;
                    case SDL_EVENT_DISPLAY_ADDED:
                    lupanic_if_failed(refresh_display_list());
                    g_display_events.connect(display);
                    break;
                    case SDL_EVENT_DISPLAY_REMOVED:
                    display->m_disconnected = true;
                    g_display_events.disconnect(display);
                    break;
                    case SDL_EVENT_DISPLAY_MOVED:
                    g_display_events.move(display);
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
                        window->get_events().show(window);
                        break;
                        case SDL_EVENT_WINDOW_HIDDEN:
                        window->get_events().hide(window);
                        break;
                        case SDL_EVENT_WINDOW_MOVED:
                        window->get_events().move(window, event.window.data1, event.window.data2);
                        break;
                        case SDL_EVENT_WINDOW_RESIZED:
                        window->get_events().resize(window, event.window.data1, event.window.data2);
                        break;
                        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                        window->get_events().framebuffer_resize(window, event.window.data1, event.window.data2);
                        break;
                        case SDL_EVENT_WINDOW_MOUSE_ENTER:
                        window->get_events().mouse_enter(window);
                        break;
                        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                        window->get_events().mouse_leave(window);
                        break;
                        case SDL_EVENT_WINDOW_FOCUS_GAINED:
                        window->get_events().input_focus(window);
                        break;
                        case SDL_EVENT_WINDOW_FOCUS_LOST:
                        window->get_events().lose_input_focus(window);
                        break;
                        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        window->get_events().close(window);
                        break;
                        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                        window->get_events().dpi_scale_changed(window);
                        break;
                        case SDL_EVENT_WINDOW_DESTROYED:
                        window->get_events().destroy(window);
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
                            window->get_events().key_down(window, key);
                        }
                        else
                        {
                            window->get_events().key_up(window, key);
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
                        window->get_events().input_character(window, ch);
                    }
                }
            }
            else if(event.type == SDL_EVENT_MOUSE_MOTION)
            {
                Window* window = get_window_from_sdl_window_id(event.motion.windowID);
                if(window)
                {
                    window->get_events().mouse_move(window, event.motion.x, event.motion.y);
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
                        window->get_events().mouse_down(window, button);
                    }
                    else
                    {
                        window->get_events().mouse_up(window, button);
                    }
                }
            }
            else if(event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                Window* window = get_window_from_sdl_window_id(event.wheel.windowID);
                if(window)
                {
                    window->get_events().scroll(window, event.wheel.x, event.wheel.y);
                }
            }
            else if(event.type == SDL_EVENT_FINGER_MOTION || event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_UP)
            {
                Window* window = get_window_from_sdl_window_id(event.tfinger.windowID);
                if(window)
                {
                    if(event.type == SDL_EVENT_FINGER_MOTION)
                    {
                        window->get_events().touch_move(window, (u64)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    }
                    else if(event.type == SDL_EVENT_FINGER_DOWN)
                    {
                        window->get_events().touch_down(window, (u64)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    }
                    else
                    {
                        window->get_events().touch_up(window, (u64)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
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
                    else if(event.type == SDL_EVENT_DROP_POSITION)
                    {
                        window->m_drop_x = event.drop.x;
                        window->m_drop_y = event.drop.y;
                    }
                    else if(event.type == SDL_EVENT_DROP_COMPLETE)
                    {
                        window->m_drop_x = event.drop.x;
                        window->m_drop_y = event.drop.y;
                        Array<const c8*> files(window->m_drop_files.size());
                        for(usize i = 0; i < window->m_drop_files.size(); ++i)
                        {
                            files[i] = window->m_drop_files[i].c_str();
                        }
                        window->get_events().drop_file(window, files.data(), files.size(), window->m_drop_x, window->m_drop_y);
                        window->m_drop_files.clear();
                    }
                }
            }
        }
    }
}