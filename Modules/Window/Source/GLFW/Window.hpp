/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2022/4/10
*/
#pragma once
#include "../../Window.hpp"

#ifdef LUNA_WINDOW_GLFW
#include <GLFW/glfw3.h>
#include "../WindowEvents.hpp"
#include "../../GLFW/GLFWWindow.hpp"

#ifdef LUNA_PLATFORM_WINDOWS
#include "../../Windows/Win32Window.hpp"
#include <Runtime/Platform/Windows/MiniWin.hpp>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace Luna
{
    namespace Window
    {
#ifdef LUNA_PLATFORM_WINDOWS
		struct Window : public IWin32Window, public IGLFWWindow
#else
		struct Window : public IGLFWWindow
#endif
        {
            lustruct("RHI::Window", "{2b85ad9e-f949-448a-8d4a-98c4ed39d537}");
			luiimpl();

            GLFWwindow* m_window;
			WindowEvents m_events;

            void close();
            bool is_closed();
			RV set_fullscreen(monitor_t monitor, u32 width, u32 height, u32 refresh_rate);
			RV unset_fullscreen(i32 x, i32 y, u32 width, u32 height);
			Int2U get_position();
			RV set_position(i32 x, i32 y);
            UInt2U get_size();
			RV set_size(u32 width, u32 height);
			UInt2U get_framebuffer_size();
            f32 get_dpi_scale_factor();
			Int2U screen_to_client(const Int2U& point);
			Int2U client_to_screen(const Int2U& point);
#ifdef LUNA_PLATFORM_WINDOWS
			virtual HWND get_hwnd() override
			{
				return glfwGetWin32Window(m_window);
			}
#endif
			GLFWwindow* get_glfw_window_handle()
			{
				return m_window;
			}
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

#endif
