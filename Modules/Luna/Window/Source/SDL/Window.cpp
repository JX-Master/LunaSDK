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
#include "Display.hpp"
#include <Luna/Runtime/Thread.hpp>
#include <Luna/HID/KeyCode.hpp>
#include <Luna/Runtime/Array.hpp>
#include "Common.hpp"

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
            bool r = SDL_RaiseWindow(m_window);
            return r ? ok : set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
        }
        bool Window::is_minimized()
        {
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MINIMIZED) != 0;
        }
        bool Window::is_maximized()
        {
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MAXIMIZED) != 0;
        }
        RV Window::set_minimized()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_MinimizeWindow(m_window);
            return encode_sdl_result(r);
        }
        RV Window::set_maximized()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_MaximizeWindow(m_window);
            return encode_sdl_result(r);
        }
        RV Window::set_restored()
        {
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_RestoreWindow(m_window);
            return encode_sdl_result(r);
        }
        bool Window::is_hovered()
        {
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MOUSE_FOCUS) != 0;
        }
        bool Window::is_visible()
        {
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_HIDDEN) == 0;
        }
        RV Window::set_visible(bool visible)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            bool r;
            if(visible)
            {
                r = SDL_ShowWindow(m_window);
            }
            else
            {
                r = SDL_HideWindow(m_window);
            }
            return encode_sdl_result(r);
        }
        bool Window::is_resizable()
        {
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_RESIZABLE) != 0;
        }
        RV Window::set_resizable(bool resizable)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_SetWindowResizable(m_window, resizable);
            return encode_sdl_result(r);
        }
        bool Window::is_borderless()
        {
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_BORDERLESS) != 0;
        }
        RV Window::set_borderless(bool borderless)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            if (is_full_screen() && !borderless) return BasicError::bad_arguments(); // Full screen window cannot be set to bordered.
            bool r = SDL_SetWindowBordered(m_window, !borderless);
            return encode_sdl_result(r);
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
            bool r = SDL_SetWindowPosition(m_window, x, y);
            return encode_sdl_result(r);
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
            bool r = SDL_SetWindowSize(m_window, width, height);
            return encode_sdl_result(r);
        }
        UInt2U Window::get_framebuffer_size()
        {
            if (is_closed()) return UInt2U(0, 0);
            int w, h;
            SDL_GetWindowSizeInPixels(m_window, &w, &h);
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
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_FULLSCREEN) != 0;
        }
        display_t Window::get_display()
        {
            if (!is_full_screen()) return nullptr;
            const SDL_DisplayMode* src_display_mode = SDL_GetWindowFullscreenMode(m_window);
            SDL_DisplayID id = src_display_mode->displayID;
            return get_display_from_display_id(id);
        }
        RV Window::set_title(const c8* title)
        {
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_SetWindowTitle(m_window, title);
            return encode_sdl_result(r);
        }
        RV Window::set_display_settings(const WindowDisplaySettings& display_settings)
        {
            lutry
            {
                if (is_closed()) return BasicError::bad_calling_time();
                if(display_settings.full_screen)
                {
                    display_t target_display = display_settings.display ? display_settings.display : get_primary_display();
                    lulet(mode, get_display_video_mode(target_display));
                    u32 width = display_settings.width == 0 ? mode.width : display_settings.width;
                    u32 height = display_settings.height == 0 ? mode.height : display_settings.height;
                    u32 refresh_rate = display_settings.refresh_rate == 0 ? mode.refresh_rate : display_settings.refresh_rate;
                    SDL_DisplayID display_id = ((Display*)target_display)->m_id;
                    luexp(encode_sdl_result(SDL_SetWindowFullscreen(m_window, true)));
                    SDL_DisplayMode display_mode;
                    const SDL_DisplayMode* src_display_mode = SDL_GetWindowFullscreenMode(m_window);
                    if(!src_display_mode) return set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
                    display_mode = *src_display_mode;
                    if( display_mode.w != width 
                        || display_mode.h != height 
                        || display_mode.refresh_rate != refresh_rate
                        || display_mode.displayID != display_id)
                    {
                        display_mode.w = width;
                        display_mode.h = height;
                        display_mode.refresh_rate = refresh_rate;
                        display_mode.displayID = display_id;
                        luexp(encode_sdl_result(SDL_SetWindowFullscreenMode(m_window, &display_mode)));
                    }
                }
                else
                {
                    luexp(encode_sdl_result(SDL_SetWindowFullscreen(m_window, false)));
                    int x = (display_settings.x == DEFAULT_POS) ? SDL_WINDOWPOS_UNDEFINED : display_settings.x;
                    int y = (display_settings.y == DEFAULT_POS) ? SDL_WINDOWPOS_UNDEFINED : display_settings.y;
                    luexp(encode_sdl_result(SDL_SetWindowPosition(m_window, x, y)));
                    lulet(mode, get_display_video_mode(get_primary_display()));
                    int w = (display_settings.width == 0) ? (u32)(mode.width * 0.7f) : display_settings.width;
                    int h = (display_settings.height == 0) ? (u32)(mode.height * 0.7f) : display_settings.height;
                    luexp(encode_sdl_result(SDL_SetWindowSize(m_window, w, h)));
                }
            }
            lucatchret;
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
        RV Window::start_text_input()
        {
            bool r = SDL_StartTextInput(m_window);
            return encode_sdl_result(r);
        }
        RV Window::stop_text_input()
        {
            bool r = SDL_StopTextInput(m_window);
            return encode_sdl_result(r);
        }
#ifdef LUNA_PLATFORM_WINDOWS
        HWND Window::get_hwnd()
        {
            if (is_closed()) return nullptr;
            HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
            return hwnd;
        }
#endif
#ifdef LUNA_PLATFORM_MACOS
        id Window::get_nswindow()
        {
            if (is_closed()) return nullptr;
            void* window = SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
            return (id)window;
        }
#endif

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
            lutry
            {
                luexp(encode_sdl_result(SDL_Init(SDL_INIT_VIDEO)));
            }
            lucatchret;
            return display_init();
        }
        void platform_close()
        {
            display_close();
            SDL_Quit();
        }

        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, const WindowDisplaySettings& display_settings, WindowCreationFlag flags)
        {
            lucheck_msg(get_current_thread() == get_main_thread(), "Window::new_window must only be called from the main thread.");
            Ref<IWindow> ret;
            SDL_PropertiesID properties = SDL_CreateProperties();
            lutry
            {
                SDL_SetStringProperty(properties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, test_flags(flags, WindowCreationFlag::resizable));
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, test_flags(flags, WindowCreationFlag::borderless));
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, test_flags(flags, WindowCreationFlag::hidden));
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, display_settings.full_screen);

                if (!display_settings.full_screen)
                {
                    // as windowed.
                    lulet(mode, get_display_video_mode(get_primary_display()));
                    if(display_settings.x != DEFAULT_POS)
                    {
                        SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_X_NUMBER, display_settings.x);
                    }
                    if(display_settings.y != DEFAULT_POS)
                    {
                        SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_Y_NUMBER, display_settings.y);
                    }
                    SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, display_settings.width == 0 ? mode.width * 7 / 10 : display_settings.width);
                    SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, display_settings.height == 0 ? mode.height * 7 / 10 : display_settings.height);
                }
                SDL_Window* sdl_window = SDL_CreateWindowWithProperties(properties);
                if (!sdl_window)
                {
                    luthrow(set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError()));
                }
                Ref<Window> window = new_object<Window>();
                window->m_window = sdl_window;
                luexp(encode_sdl_result(SDL_SetPointerProperty(SDL_GetWindowProperties(sdl_window), "LunaWindow", window->get_object())));
                // switch the window to the correct display mode if on full-screen mode.
                if (display_settings.full_screen)
                {
                    luexp(window->set_display_settings(display_settings));
                }
                ret = window;
            }
            lucatch
            {
                if(properties)
                {
                    SDL_DestroyProperties(properties);
                    properties = 0;
                }
                return luerr;
            }
            if(properties)
            {
                SDL_DestroyProperties(properties);
                properties = 0;
            }
            return ret;
        }

        LUNA_WINDOW_API void set_startup_params(const StartupParams& params)
        {
            g_startup_params = params;
        }

        LUNA_WINDOW_API void dispatch_window_close_event(IWindow* window)
        {
            window->get_events().close(window);
        }

        LUNA_WINDOW_API void dispatch_window_focus_event(IWindow* window)
        {
            window->get_events().focus(window);
        }

        LUNA_WINDOW_API void dispatch_window_lose_focus_event(IWindow* window)
        {
            window->get_events().lose_focus(window);
        }

        LUNA_WINDOW_API void dispatch_window_show_event(IWindow* window)
        {
            window->get_events().show(window);
        }

        LUNA_WINDOW_API void dispatch_window_hide_event(IWindow* window)
        {
            window->get_events().hide(window);
        }

        LUNA_WINDOW_API void dispatch_window_resize_event(IWindow* window, u32 width, u32 height)
        {
            window->get_events().resize(window, width, height);
        }

        LUNA_WINDOW_API void dispatch_window_framebuffer_resize_event(IWindow* window, u32 width, u32 height)
        {
            window->get_events().framebuffer_resize(window, width, height);
        }

        LUNA_WINDOW_API void dispatch_window_move_event(IWindow* window, i32 x, i32 y)
        {
            window->get_events().move(window, x, y);
        }

        LUNA_WINDOW_API void dispatch_window_dpi_changed_event(IWindow* window)
        {
            window->get_events().dpi_changed(window);
        }

        LUNA_WINDOW_API void dispatch_window_key_down_event(IWindow* window, HID::KeyCode key)
        {
            window->get_events().key_down(window, key);
        }

        LUNA_WINDOW_API void dispatch_window_key_up_event(IWindow* window, HID::KeyCode key)
        {
            window->get_events().key_up(window, key);
        }

        LUNA_WINDOW_API void dispatch_window_input_character_event(IWindow* window, c32 character)
        {
            window->get_events().input_character(window, character);
        }

        LUNA_WINDOW_API void dispatch_window_mouse_enter_event(IWindow* window)
        {
            window->get_events().mouse_enter(window);
        }

        LUNA_WINDOW_API void dispatch_window_mouse_leave_event(IWindow* window)
        {
            window->get_events().mouse_leave(window);
        }

        LUNA_WINDOW_API void dispatch_window_mouse_move_event(IWindow* window, i32 x, i32 y)
        {
            window->get_events().mouse_move(window, x, y);
        }

        LUNA_WINDOW_API void dispatch_window_mouse_down_event(IWindow* window, HID::MouseButton button)
        {
            window->get_events().mouse_down(window, button);
        }

        LUNA_WINDOW_API void dispatch_window_mouse_up_event(IWindow* window, HID::MouseButton button)
        {
            window->get_events().mouse_up(window, button);
        }

        LUNA_WINDOW_API void dispatch_window_scroll_event(IWindow* window, f32 scroll_x, f32 scroll_y)
        {
            window->get_events().scroll(window, scroll_x, scroll_y);
        }

        LUNA_WINDOW_API void dispatch_window_touch_move_event(IWindow* window, u64 id, f32 x, f32 y)
        {
            window->get_events().touch_move(window, id, x, y);
        }

        LUNA_WINDOW_API void dispatch_window_touch_down_event(IWindow* window, u64 id, f32 x, f32 y)
        {
            window->get_events().touch_down(window, id, x, y);
        }

        LUNA_WINDOW_API void dispatch_window_touch_up_event(IWindow* window, u64 id, f32 x, f32 y)
        {
            window->get_events().touch_up(window, id, x, y);
        }

        LUNA_WINDOW_API void dispatch_window_drop_file_event(IWindow* window, const c8** paths, usize num_paths)
        {
            window->get_events().drop_file(window, paths, num_paths);
        }
    }
}