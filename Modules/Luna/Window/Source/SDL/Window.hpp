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

namespace Luna
{
    namespace Window
    {
        struct Window : public IWindow
        {
            lustruct("Window::Window", "{757e4968-d2f8-45aa-90ff-93e59d921c19}");
            luiimpl();

            SDL_Window* m_window;
            WindowEvents m_events;

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
            virtual RV set_visible(bool visible);
            virtual bool is_resizable() override;
            virtual RV set_resizable(bool resizable);
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
            virtual RV set_title(const c8* title);
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
            virtual Event<window_begin_resize_move_t>& get_begin_resize_move_event()  override { return m_events.begin_resize_move; }
            virtual Event<window_end_resize_move_t>& get_end_resize_move_event()  override { return m_events.end_resize_move; }
            virtual Event<window_dpi_changed_event_handler_t>& get_dpi_changed_event()  override { return m_events.dpi_changed; }
            virtual Event<window_key_down_event_handler_t>& get_key_down_event()  override { return m_events.key_down; }
            virtual Event<window_key_up_event_handler_t>& get_key_up_event()  override { return m_events.key_up; }
            virtual Event<window_input_character_event_handler_t>& get_input_character_event()  override { return m_events.input_character; }
            virtual Event<window_mouse_move_event_handler_t>& get_mouse_move_event()  override { return m_events.mouse_move; }
            virtual Event<window_mouse_down_event_handler_t>& get_mouse_down_event()  override { return m_events.mouse_down; }
            virtual Event<window_mouse_up_event_handler_t>& get_mouse_up_event()  override { return m_events.mouse_up; }
            virtual Event<window_mouse_wheel_event_handler_t>& get_mouse_wheel_event()  override { return m_events.mouse_wheel; }
            virtual Event<window_touch_event_handler_t>& get_touch_event()  override { return m_events.touch; }
            virtual Event<window_drop_file_event_handler_t>& get_drop_file_event()  override { return m_events.drop_file; }
            Window() :
                m_window(nullptr) {}
            ~Window()
            {
                close();
            }
        };
    }
}