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
#include "../WindowEvents.hpp"

#include <SDL.h>

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
            // Used to trigger resize event when new size does not match cached size.
            UInt2U m_event_cached_size;
            UInt2U m_event_cached_framebuffer_size;
            f32 m_event_cached_dpi;
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
            virtual bool is_frameless() override;
            virtual RV set_frameless(bool frameless) override;
            virtual Int2U get_position() override;
            virtual RV set_position(i32 x, i32 y) override;
            virtual UInt2U get_size() override;
            virtual RV set_size(u32 width, u32 height) override;
            virtual UInt2U get_framebuffer_size() override;
            virtual f32 get_dpi_scale_factor() override;
            virtual bool is_full_screen() override;
            virtual monitor_t get_monitor() override;
            virtual RV set_title(const c8* title) override;
            virtual RV set_display_settings(const WindowDisplaySettings& display_settings) override;
            virtual Int2U screen_to_client(const Int2U& point) override;
            virtual Int2U client_to_screen(const Int2U& point) override;
            virtual Event<window_close_event_handler_t>& get_close_event()  override { return m_events.close; }
            virtual Event<window_focus_event_handler_t>& get_focus_event()  override { return m_events.focus; }
            virtual Event<window_lose_focus_event_handler_t>& get_lose_focus_event()  override { return m_events.lose_focus; }
            virtual Event<window_show_event_handler_t>& get_show_event()  override { return m_events.show; }
            virtual Event<window_hide_event_handler_t>& get_hide_event()  override { return m_events.hide; }
            virtual Event<window_resize_event_handler_t>& get_resize_event()  override { return m_events.resize; }
            virtual Event<window_framebuffer_resize_event_handler_t>& get_framebuffer_resize_event() override { return m_events.framebuffer_resize; }
            virtual Event<window_move_event_handler_t>& get_move_event()  override { return m_events.move; }
            virtual Event<window_dpi_changed_event_handler_t>& get_dpi_changed_event()  override { return m_events.dpi_changed; }
            virtual Event<window_key_down_event_handler_t>& get_key_down_event()  override { return m_events.key_down; }
            virtual Event<window_key_up_event_handler_t>& get_key_up_event()  override { return m_events.key_up; }
            virtual Event<window_input_character_event_handler_t>& get_input_character_event()  override { return m_events.input_character; }
            virtual Event<window_mouse_enter_event_handler_t>& get_mouse_enter_event() override { return m_events.mouse_enter; }
            virtual Event<window_mouse_leave_event_handlet_t>& get_mouse_leave_event() override { return m_events.mouse_leave; }
            virtual Event<window_mouse_move_event_handler_t>& get_mouse_move_event()  override { return m_events.mouse_move; }
            virtual Event<window_mouse_down_event_handler_t>& get_mouse_down_event()  override { return m_events.mouse_down; }
            virtual Event<window_mouse_up_event_handler_t>& get_mouse_up_event()  override { return m_events.mouse_up; }
            virtual Event<window_mouse_wheel_event_handler_t>& get_mouse_wheel_event()  override { return m_events.mouse_wheel; }
            virtual Event<window_touch_move_event_handler_t>& get_touch_move_event()  override { return m_events.touch_move; }
            virtual Event<window_touch_move_event_handler_t>& get_touch_down_event()  override { return m_events.touch_down; }
            virtual Event<window_touch_move_event_handler_t>& get_touch_up_event()  override { return m_events.touch_up; }
            virtual Event<window_drop_file_event_handler_t>& get_drop_file_event()  override { return m_events.drop_file; }
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
        };
    }
}