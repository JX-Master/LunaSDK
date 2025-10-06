/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.cpp
* @author JXMaster
* @date 2024/6/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#include <SDL3/SDL_keyboard.h>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "Window.hpp"
#include "../../Window.hpp"
#include "Display.hpp"
#include <Luna/Runtime/Thread.hpp>
#include <Luna/HID/KeyCode.hpp>
#include <Luna/Runtime/Array.hpp>
#include "Common.hpp"
#include <Luna/Runtime/TSAssert.hpp>

#if defined(LUNA_PLATFORM_MACOS) || defined(LUNA_PLATFORM_IOS)

#endif

namespace Luna
{
    namespace Window
    {
        void Window::close()
        {
            lutsassert_main_thread();
            if(m_window)
            {
                SDL_DestroyWindow(m_window);
                m_window = nullptr;
            }
        }
        bool Window::is_closed()
        {
            lutsassert_main_thread();
            return m_window == nullptr;
        }
        bool Window::has_input_focus()
        {
            lutsassert_main_thread();
            return m_window && SDL_GetKeyboardFocus() == m_window;
        }
        bool Window::has_mouse_focus()
        {
            lutsassert_main_thread();
            return m_window && SDL_GetMouseFocus() == m_window;
        }
        RV Window::set_foreground()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_RaiseWindow(m_window);
            return r ? ok : set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError());
        }
        bool Window::is_minimized()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MINIMIZED) != 0;
        }
        bool Window::is_maximized()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MAXIMIZED) != 0;
        }
        RV Window::set_minimized()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_MinimizeWindow(m_window);
            return encode_sdl_result(r);
        }
        RV Window::set_maximized()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_MaximizeWindow(m_window);
            return encode_sdl_result(r);
        }
        RV Window::set_restored()
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_RestoreWindow(m_window);
            return encode_sdl_result(r);
        }
        bool Window::is_hovered()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_MOUSE_FOCUS) != 0;
        }
        bool Window::is_visible()
        {
            lutsassert_main_thread();
            if (is_closed()) return false;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return (flags & SDL_WINDOW_HIDDEN) == 0;
        }
        RV Window::set_visible(bool visible)
        {
            lutsassert_main_thread();
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
        inline WindowStyleFlag decode_style(SDL_WindowFlags flags)
        {
            WindowStyleFlag style = WindowStyleFlag::none;
            if(flags & SDL_WINDOW_BORDERLESS)
            {
                set_flags(style, WindowStyleFlag::borderless);
            }
            if(flags & SDL_WINDOW_RESIZABLE)
            {
                set_flags(style, WindowStyleFlag::resizable);
            }
            return style;
        }
        WindowStyleFlag Window::get_style()
        {
            lutsassert_main_thread();
            if (is_closed()) return WindowStyleFlag::none;
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            return decode_style(flags);
        }
        RV Window::set_style(WindowStyleFlag style)
        {
            lutsassert_main_thread();
            bool r = SDL_SetWindowResizable(m_window, test_flags(style, WindowStyleFlag::resizable));
            if(!r)
            {
                return encode_sdl_result(r);
            }
            r = SDL_SetWindowBordered(m_window, !test_flags(style, WindowStyleFlag::borderless));
            if(!r)
            {
                return encode_sdl_result(r);
            }
            return ok;
        }
        Int2U Window::get_position()
        {
            lutsassert_main_thread();
            if (is_closed()) return Int2U(0, 0);
            int x, y;
            SDL_GetWindowPosition(m_window, &x, &y);
            return Int2U(x, y);
        }
        RV Window::set_position(i32 x, i32 y)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_SetWindowPosition(m_window, x, y);
            return encode_sdl_result(r);
        }
        UInt2U Window::get_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            int w, h;
            SDL_GetWindowSize(m_window, &w, &h);
            return UInt2U((u32)w, (u32)h);
        }
        RV Window::set_size(u32 width, u32 height)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_SetWindowSize(m_window, width, height);
            return encode_sdl_result(r);
        }
        UInt2U Window::get_framebuffer_size()
        {
            lutsassert_main_thread();
            if (is_closed()) return UInt2U(0, 0);
            int w, h;
            SDL_GetWindowSizeInPixels(m_window, &w, &h);
            return UInt2U((u32)w, (u32)h);
        }
        f32 Window::get_dpi_scale_factor()
        {
            lutsassert_main_thread();
            if (is_closed()) return 1.0f;
            UInt2U fs = get_framebuffer_size();
            UInt2U ws = get_size();
            f64 dpix = (f64)fs.x / (f64)ws.x;
            f64 dpiy = (f64)fs.y / (f64)ws.y;
            // On most cases dpix == dpiy, but if it is not, we use diagonal size ratio
            // as DPI ratio.
            return (f32)sqrt((dpix * dpix + dpiy * dpiy) / 2);
        }
        RV Window::set_title(const c8* title)
        {
            lutsassert_main_thread();
            if (is_closed()) return BasicError::bad_calling_time();
            bool r = SDL_SetWindowTitle(m_window, title);
            return encode_sdl_result(r);
        }
        Int2U Window::screen_to_client(const Int2U& point)
        {
            lutsassert_main_thread();
            auto pos = get_position();
            return Int2U(point.x - pos.x, point.y - pos.y);
        }
        Int2U Window::client_to_screen(const Int2U& point)
        {
            lutsassert_main_thread();
            auto pos = get_position();
            return Int2U(point.x + pos.x, point.y + pos.y);
        }
        RV Window::begin_text_input()
        {
            lutsassert_main_thread();
            bool r = SDL_StartTextInput(m_window);
            return encode_sdl_result(r);
        }
        RV Window::set_text_input_area(const RectI& input_rect, i32 cursor)
        {
            lutsassert_main_thread();
            SDL_Rect rect;
            rect.x = input_rect.offset_x;
            rect.y = input_rect.offset_y;
            rect.w = input_rect.width;
            rect.h = input_rect.height;
            bool r = SDL_SetTextInputArea(m_window, &rect, cursor);
            return encode_sdl_result(r);
        }
        RV Window::end_text_input()
        {
            lutsassert_main_thread();
            bool r = SDL_StopTextInput(m_window);
            return encode_sdl_result(r);
        }
#ifdef LUNA_PLATFORM_WINDOWS
        HWND Window::get_hwnd()
        {
            lutsassert_main_thread();
            if (is_closed()) return nullptr;
            HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
            return hwnd;
        }
#endif
#ifdef LUNA_PLATFORM_MACOS
        id Window::get_nswindow()
        {
            lutsassert_main_thread();
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

        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, 
            i32 x,
            i32 y,
            u32 width,
            u32 height,
            WindowStyleFlag style_flags,
            WindowCreationFlag flags)
        {
            lutsassert_main_thread();
            Ref<IWindow> ret;
            SDL_PropertiesID properties = SDL_CreateProperties();
            lutry
            {
                SDL_SetStringProperty(properties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, test_flags(style_flags, WindowStyleFlag::resizable));
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, test_flags(style_flags, WindowStyleFlag::borderless));
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, test_flags(flags, WindowCreationFlag::hidden));
                SDL_SetBooleanProperty(properties, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, false);

                lulet(mode, get_display_video_mode(get_primary_display()));
                if(x != DEFAULT_POS)
                {
                    SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
                }
                if(y != DEFAULT_POS)
                {
                    SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
                }
                SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width == 0 ? mode.width * 7 / 10 : width);
                SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height == 0 ? mode.height * 7 / 10 : height);
                SDL_Window* sdl_window = SDL_CreateWindowWithProperties(properties);
                if (!sdl_window)
                {
                    luthrow(set_error(BasicError::bad_platform_call(), "SDL error: %s", SDL_GetError()));
                }
                Ref<Window> window = new_object<Window>();
                window->m_window = sdl_window;
                luexp(encode_sdl_result(SDL_SetPointerProperty(SDL_GetWindowProperties(sdl_window), "LunaWindow", window->get_object())));
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
    }
}