/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2024/6/16
*/
#pragma once
#include "../../Window.hpp"

#include <SDL3/SDL.h>

#ifdef LUNA_PLATFORM_WINDOWS
#include "../../Windows/Win32Window.hpp"
#endif

#ifdef LUNA_PLATFORM_MACOS
#include "../../Cocoa/CocoaWindow.hpp"
#endif

namespace Luna
{
    namespace Window
    {
#if defined(LUNA_PLATFORM_WINDOWS)
        struct Window : public IWin32Window
#elif defined(LUNA_PLATFORM_MACOS)
        struct Window : public ICocoaWindow
#else
        struct Window : public IWindow
#endif
        {
            lustruct("Window::Window", "{757e4968-d2f8-45aa-90ff-93e59d921c19}");
            luiimpl();

            SDL_Window* m_window;
            WindowEvents m_events;
            // Used to cache files specified by SDL_DROPFILE.
            Vector<String> m_drop_files;

            virtual void close() override;
            virtual bool is_closed() override;
            virtual bool is_focused() override;
            virtual RV set_focus() override;
            virtual bool is_minimized() override;
            virtual bool is_maximized() override;
            virtual RV set_minimized() override;
            virtual RV set_maximized() override;
            virtual RV set_restored() override;
            virtual bool is_hovered() override;
            virtual bool is_visible() override;
            virtual RV set_visible(bool visible) override;
            virtual bool is_resizable() override;
            virtual RV set_resizable(bool resizable) override;
            virtual bool is_borderless() override;
            virtual RV set_borderless(bool borderless) override;
            virtual Int2U get_position() override;
            virtual RV set_position(i32 x, i32 y) override;
            virtual UInt2U get_size() override;
            virtual RV set_size(u32 width, u32 height) override;
            virtual UInt2U get_framebuffer_size() override;
            virtual f32 get_dpi_scale_factor() override;
            virtual bool is_full_screen() override;
            virtual display_t get_display() override;
            virtual RV set_title(const c8* title) override;
            virtual RV set_display_settings(const WindowDisplaySettings& display_settings) override;
            virtual Int2U screen_to_client(const Int2U& point) override;
            virtual Int2U client_to_screen(const Int2U& point) override;
            virtual WindowEvents& get_events() override
            {
                return m_events;
            }
#ifdef LUNA_PLATFORM_WINDOWS
            virtual HWND get_hwnd() override;
#endif
#ifdef LUNA_PLATFORM_MACOS
            virtual id get_nswindow() override;
#endif
            Window() :
                m_window(nullptr) {}
            ~Window()
            {
                close();
            }
            virtual RV start_text_input() override;
            virtual RV stop_text_input() override;
        };
    }
}