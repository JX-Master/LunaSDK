/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2024/6/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include "../Window.hpp"
#include "Monitor.hpp"
#include <Luna/Runtime/Thread.hpp>
#include <Luna/HID/KeyCode.hpp>
#include <Luna/Runtime/Array.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include <SDL_syswm.h>

#if defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)

#endif

namespace Luna
{
    namespace Window
    {
        void Window::close()
        {
            if(m_window)
            {
                SDL_DestroyWindow(m_window);
                m_window = nullptr;
            }
        }
        bool Window::is_closed()
        {
            return m_window == nullptr;
        }
        bool Window::is_focused()
        {
            return SDL_GetKeyboardFocus() == m_window;
        }
        RV Window::set_focus()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            SDL_RaiseWindow(m_window);
            return ok;
        }
        bool Window::is_minimized()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MINIMIZED) != 0;
        }
        bool Window::is_maximized()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MAXIMIZED) != 0;
        }
        RV Window::set_minimized()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            SDL_MinimizeWindow(m_window);
            return ok;
        }
        RV Window::set_maximized()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            SDL_MaximizeWindow(m_window);
            return ok;
        }
        RV Window::set_restored()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            SDL_RestoreWindow(m_window);
            return ok;
        }
        bool Window::is_hovered()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MOUSE_FOCUS) != 0;
        }
        bool Window::is_visible()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_SHOWN) != 0;
        }
        RV Window::set_visible(bool visible)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            if(visible)
            {
                SDL_ShowWindow(m_window);
            }
            else
            {
                SDL_HideWindow(m_window);
            }
            return ok;
        }
        bool Window::is_resizable()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_RESIZABLE) != 0;
        }
        RV Window::set_resizable(bool resizable)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            SDL_SetWindowResizable(m_window, resizable ? SDL_TRUE : SDL_FALSE);
            return ok;
        }
        bool Window::is_frameless()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_BORDERLESS) != 0;
        }
        RV Window::set_frameless(bool frameless)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            if (is_full_screen() && !frameless) return BasicError::bad_arguments(); // Full screen window cannot be set to bordered.
            SDL_SetWindowBordered(m_window, frameless ? SDL_FALSE : SDL_TRUE);
            return ok;
        }
        Int2U Window::get_position()
        {
            if (is_closed()) return Int2U(0, 0);
            int x, y;
            SDL_GetWindowPosition(m_window, &x, &y);
            return Int2U(x, y);
        }
        RV Window::set_position(i32 x, i32 y)
        {
            if (is_closed() || is_full_screen()) return BasicError::bad_calling_time();
            SDL_SetWindowPosition(m_window, x, y);
            return ok;
        }
        UInt2U Window::get_size()
        {
            if (is_closed()) return UInt2U(0, 0);
            int w, h;
            SDL_GetWindowSize(m_window, &w, &h);
            return UInt2U((u32)w, (u32)h);
        }
        RV Window::set_size(u32 width, u32 height)
        {
            if (is_closed() || is_full_screen()) return BasicError::bad_calling_time();
            SDL_SetWindowSize(m_window, width, height);
            return ok;
        }
        UInt2U Window::get_framebuffer_size()
        {
            if (is_closed()) return UInt2U(0, 0);
            int w, h;
#ifdef LUNA_PLATFORM_WINDOWS
            SDL_GetWindowSize(m_window, &w, &h);
#elif defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)
            SDL_Metal_GetDrawableSize(m_window, &w, &h);
#endif
            return UInt2U((u32)w, (u32)h);
        }
        f32 Window::get_dpi_scale_factor()
        {
            if (is_closed()) return 1.0f;
            UInt2U fs = get_framebuffer_size();
            UInt2U ws = get_size();
            f64 dpix = (f64)fs.x / (f64)ws.x;
            f64 dpiy = (f64)fs.y / (f64)ws.y;
            // On most cases dpix == dpiy, but if it is not, we use diagonal size ratio
            // as DPI ratio.
            return (f32)sqrt((dpix * dpix + dpiy * dpiy) / 2);
        }
        bool Window::is_full_screen()
        {
            if (is_closed()) return false;
            u32 flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_FULLSCREEN) != 0;
        }
        monitor_t Window::get_monitor()
        {
            if (!is_full_screen()) return nullptr;
            // SDL2 cannot set fullscreen on a secondary monitor, we will fix this in SDL3.
            return get_primary_monitor();
        }
        RV Window::set_title(const c8* title)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            SDL_SetWindowTitle(m_window, title);
            return ok;
        }
        RV Window::set_display_settings(const WindowDisplaySettings& display_settings)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            if(display_settings.full_screen)
            {
                u32 flags = SDL_WINDOW_FULLSCREEN;
                VideoMode mode = get_monitor_video_mode(get_primary_monitor());
                u32 width = display_settings.width == 0 ? mode.width : display_settings.width;
                u32 height = display_settings.height == 0 ? mode.height : display_settings.height;
                u32 refresh_rate = display_settings.refresh_rate == 0 ? mode.refresh_rate : display_settings.refresh_rate;
                if (width == mode.width && height == mode.height && refresh_rate == mode.refresh_rate)
                {
                    flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
                }
                int r = SDL_SetWindowFullscreen(m_window, flags);
                if(r < 0) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
                SDL_DisplayMode display_mode;
                r = SDL_GetWindowDisplayMode(m_window, &display_mode);
                if(r < 0) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
                if(display_mode.w != width || display_mode.h != height || display_mode.refresh_rate != refresh_rate)
                {
                    display_mode.w = width;
                    display_mode.h = height;
                    display_mode.refresh_rate = refresh_rate;
                    r = SDL_SetWindowDisplayMode(m_window, &display_mode);
                    if(r < 0) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
                }
            }
            else
            {
                int r = SDL_SetWindowFullscreen(m_window, 0);
                if(r < 0) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
                int x = (display_settings.x == DEFAULT_POS) ? SDL_WINDOWPOS_UNDEFINED : display_settings.x;
                int y = (display_settings.y == DEFAULT_POS) ? SDL_WINDOWPOS_UNDEFINED : display_settings.y;
                SDL_SetWindowPosition(m_window, x, y);
                VideoMode mode = get_monitor_video_mode(get_primary_monitor());
                int w = (display_settings.width == 0) ? (u32)(mode.width * 0.7f) : display_settings.width;
                int h = (display_settings.height == 0) ? (u32)(mode.height * 0.7f) : display_settings.height;
                SDL_SetWindowSize(m_window, w, h);
            }
            return ok;
        }
        Int2U Window::screen_to_client(const Int2U& point)
        {
            auto pos = get_position();
            return Int2U(point.x - pos.x, point.y - pos.y);
        }
        Int2U Window::client_to_screen(const Int2U& point)
        {
            auto pos = get_position();
            return Int2U(point.x + pos.x, point.y + pos.y);
        }
#ifdef LUNA_PLATFORM_WINDOWS
        HWND Window::get_hwnd()
        {
            if (is_closed()) return nullptr;
            SDL_SysWMinfo info;
            SDL_bool r = SDL_GetWindowWMInfo(m_window, &info);
            luassert(info.subsystem == SDL_SYSWM_WINDOWS);
            if (r == SDL_FALSE) return nullptr;
            return info.info.win.window;
        }
#endif
#ifdef LUNA_PLATFORM_MACOS
        id Window::get_nswindow()
        {
            if (is_closed()) return nullptr;
            SDL_SysWMinfo info;
            SDL_bool r = SDL_GetWindowWMInfo(m_window, &info);
            luassert(info.subsystem == SDL_SYSWM_COCOA);
            if (r == SDL_FALSE) return nullptr;
            return (id)info.info.cocoa.window;
        }
#endif
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

        RV platform_init()
        {
            register_boxed_type<Window>();
#if defined(LUNA_PLATFORM_WINDOWS)
            impl_interface_for_type<Window, IWin32Window, IWindow>();
#elif defined(LUNA_PLATFORM_MACOS)
            impl_interface_for_type<Window, ICocoaWindow, IWindow>();
#else
            impl_interface_for_type<Window, IWindow>();
#endif
            if (SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                return set_error(BasicError::bad_platform_call(), "SDL initialization failed: %s", SDL_GetError());
            }
            return monitor_init();
        }
        void platform_close()
        {
            monitor_close();
            SDL_Quit();
        }

        static void handle_window_resize_event(Window* window)
        {
            UInt2U new_size = window->get_size();
            if (new_size != window->m_event_cached_size)
            {
                window->m_event_cached_size = new_size;
                window->m_events.resize(window, new_size.x, new_size.y);
            }
            UInt2U new_framebuffer_size = window->get_framebuffer_size();
            if (new_framebuffer_size != window->m_event_cached_framebuffer_size)
            {
                window->m_event_cached_framebuffer_size = new_framebuffer_size;
                window->m_events.framebuffer_resize(window, new_framebuffer_size.x, new_framebuffer_size.y);
            }
            f32 new_dpi = window->get_dpi_scale_factor();
            if (new_dpi != window->m_event_cached_dpi)
            {
                window->m_event_cached_dpi = new_dpi;
                window->m_events.dpi_changed(window, new_dpi);
            }
        }

        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            SDL_Event event;
            int any_event = 0;
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
                switch(event.type)
                {
                    case SDL_DISPLAYEVENT:
                    {
                        MonitorEvent e;
                        e.orientation = MonitorOrientation::unknown;
                        switch(event.display.event)
                        {
                            case SDL_DISPLAYEVENT_ORIENTATION:
                            e.type = MonitorEventType::orientation;
                            switch(event.display.data1)
                            {
                                case SDL_ORIENTATION_UNKNOWN:
                                e.orientation = MonitorOrientation::unknown;
                                break;
                                case SDL_ORIENTATION_LANDSCAPE:
                                e.orientation = MonitorOrientation::landscape;
                                break;
                                case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                                e.orientation = MonitorOrientation::landscape_flipped;
                                break;
                                case SDL_ORIENTATION_PORTRAIT:
                                e.orientation = MonitorOrientation::portrait;
                                break;
                                case SDL_ORIENTATION_PORTRAIT_FLIPPED:
                                e.orientation = MonitorOrientation::portrait_flipped;
                                break;
                            }
                            break;
                            case SDL_DISPLAYEVENT_CONNECTED:
                            e.type = MonitorEventType::connected;
                            break;
                            case SDL_DISPLAYEVENT_DISCONNECTED:
                            {
                                e.type = MonitorEventType::disconnected;
                                Monitor* m = (Monitor*)get_monitor(event.display.display);
                                m->m_disconnected = true;
                            }
                            break;
                            case SDL_DISPLAYEVENT_MOVED:
                            e.type = MonitorEventType::moved;
                            break;
                            default:
                            lupanic();
                            break;
                        }
                        if (e.type == MonitorEventType::connected)
                        {
                            lupanic_if_failed(refresh_monitor_list());
                        }
                        dispatch_monitor_event(get_monitor(event.display.display), e);
                        if (e.type == MonitorEventType::disconnected)
                        {
                            lupanic_if_failed(refresh_monitor_list());
                        }
                    }
                    break;
                    case SDL_WINDOWEVENT:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.window.windowID);
                        Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                        switch(event.window.event)
                        {
                            case SDL_WINDOWEVENT_SHOWN:
                            window->m_events.show(window);
                            break;
                            case SDL_WINDOWEVENT_HIDDEN:
                            window->m_events.hide(window);
                            break;
                            case SDL_WINDOWEVENT_MOVED:
                            window->m_events.move(window, event.window.data1, event.window.data2);
                            break;
                            case SDL_WINDOWEVENT_RESIZED:
                            handle_window_resize_event(window);
                            break;
                            case SDL_WINDOWEVENT_ENTER:
                            window->m_events.mouse_enter(window);
                            break;
                            case SDL_WINDOWEVENT_LEAVE:
                            window->m_events.mouse_leave(window);
                            break;
                            case SDL_WINDOWEVENT_FOCUS_GAINED:
                            window->m_events.focus(window);
                            break;
                            case SDL_WINDOWEVENT_FOCUS_LOST:
                            window->m_events.lose_focus(window);
                            break;
                            case SDL_WINDOWEVENT_CLOSE:
                            window->m_events.close(window);
                            break;
                            case SDL_WINDOWEVENT_DISPLAY_CHANGED:
                            handle_window_resize_event(window);
                        }
                    }
                    break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.key.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                            HID::KeyCode key = translate_key(event.key.keysym.scancode);
                            if (key != HID::KeyCode::unknown)
                            {
                                if (event.type == SDL_KEYDOWN)
                                {
                                    window->m_events.key_down(window, key);
                                }
                                else
                                {
                                    window->m_events.key_up(window, key);
                                }
                            }
                        }
                    }
                    break;
                    case SDL_TEXTINPUT:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.text.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                            usize utf8_len = utf8_strlen(event.text.text);
                            const char* cur = event.text.text;
                            for(usize i = 0; i < utf8_len; ++i)
                            {
                                c32 ch = utf8_decode_char(cur);
                                cur += utf8_charspan(ch);
                                window->m_events.input_character(window, ch);
                            }
                        }
                    }
                    break;
                    case SDL_MOUSEMOTION:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.motion.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                            window->m_events.mouse_move(window, event.motion.x, event.motion.y);
                        }
                    }
                    break;
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.button.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
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
                            if(event.type == SDL_MOUSEBUTTONDOWN)
                            {
                                window->m_events.mouse_down(window, button);
                            }
                            else
                            {
                                window->m_events.mouse_up(window, button);
                            }
                        }
                    }
                    break;
                    case SDL_MOUSEWHEEL:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.wheel.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                            window->m_events.mouse_wheel(window, event.wheel.preciseX, event.wheel.preciseY);
                        }
                    }
                    break;
                    case SDL_FINGERMOTION:
                    case SDL_FINGERDOWN:
                    case SDL_FINGERUP:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.tfinger.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                            if(event.type == SDL_FINGERMOTION)
                            {
                                window->m_events.touch_move(window, (u64)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
                            }
                            else if(event.type == SDL_FINGERDOWN)
                            {
                                window->m_events.touch_down(window, (u64)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
                            }
                            else
                            {
                                window->m_events.touch_up(window, (u64)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
                            }
                        }
                    }
                    break;
                    case SDL_DROPFILE:
                    case SDL_DROPBEGIN:
                    case SDL_DROPCOMPLETE:
                    {
                        SDL_Window* sdl_window = SDL_GetWindowFromID(event.drop.windowID);
                        if(sdl_window)
                        {
                            Window* window = (Window*)SDL_GetWindowData(sdl_window, "LunaWindow");
                            
                            if(event.type == SDL_DROPFILE)
                            {
                                window->m_drop_files.push_back(event.drop.file);
                            }
                            else if(event.type == SDL_DROPCOMPLETE)
                            {
                                Array<const c8*> files(window->m_drop_files.size());
                                for(usize i = 0; i < window->m_drop_files.size(); ++i)
                                {
                                    files[i] = window->m_drop_files[i].c_str();
                                }
                                window->m_events.drop_file(window, files.span());
                                window->m_drop_files.clear();
                            }
                        }
                        // According to SDL docs, the filename should be freed manually.
                        if (event.drop.file)
                        {
                            SDL_free(event.drop.file);
                            event.drop.file = nullptr;
                        }
                    }
                    break;
                }
                any_event = SDL_PollEvent(&event);
            }
        }
        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, const WindowDisplaySettings& display_settings, WindowCreationFlag flags)
        {
            lucheck_msg(get_current_thread() == get_main_thread(), "RHI::new_window must only be called from the main thread.");
            u32 window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
            if (test_flags(flags, WindowCreationFlag::resizable))
            {
                window_flags |= SDL_WINDOW_RESIZABLE;
            }
            if (test_flags(flags, WindowCreationFlag::borderless))
            {
                window_flags |= SDL_WINDOW_BORDERLESS;
            }
            if (test_flags(flags, WindowCreationFlag::hidden))
            {
                window_flags |= SDL_WINDOW_HIDDEN;
            }
            int x = SDL_WINDOWPOS_UNDEFINED;
            int y = SDL_WINDOWPOS_UNDEFINED;
            int w = 0;
            int h = 0;
            if (display_settings.full_screen)
            {
                // as full screen.
                window_flags |= SDL_WINDOW_FULLSCREEN;
                if (display_settings.width == 0 && display_settings.height == 0)
                {
                    window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                }
            }
            else
            {
                // as windowed.
                VideoMode mode = get_monitor_video_mode(get_primary_monitor());
                if (display_settings.x != DEFAULT_POS) x = display_settings.x;
                if (display_settings.y != DEFAULT_POS) y = display_settings.y;
                w = display_settings.width == 0 ? mode.width * 7 / 10 : display_settings.width;
                h = display_settings.height == 0 ? mode.height * 7 / 10 : display_settings.height;
            }
            SDL_Window* sdl_window = SDL_CreateWindow(title, x, y, w, h, window_flags);
            if (!sdl_window)
            {
                return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
            }
            Ref<Window> window = new_object<Window>();
            window->m_window = sdl_window;
            SDL_SetWindowData(sdl_window, "LunaWindow", window->get_object());
            window->m_event_cached_size = window->get_size();
            window->m_event_cached_framebuffer_size = window->get_framebuffer_size();
            window->m_event_cached_dpi = window->get_dpi_scale_factor();
            // switch the window to the correct display mode if on full-screen mode.
            if (display_settings.full_screen)
            {
                auto r = window->set_display_settings(display_settings);
                if (failed(r)) return r.errcode();
            }
            return Ref<IWindow>(window);
        }

        LUNA_WINDOW_API void set_startup_params(const StartupParams& params)
        {
            g_startup_params = params;
        }
    }
}