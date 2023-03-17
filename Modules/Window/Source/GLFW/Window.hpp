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
		inline ErrCode encode_glfw_error()
		{
			const char* description;
			int code = glfwGetError(&description);
			if (code != GLFW_NO_ERROR)
			{
				ErrCode err_code;
				switch (code)
				{
				case GLFW_NOT_INITIALIZED: err_code = BasicError::bad_calling_time(); break;
				case GLFW_NO_CURRENT_CONTEXT: err_code = BasicError::bad_platform_call(); break;
				case GLFW_INVALID_ENUM: err_code = BasicError::bad_arguments(); break;
				case GLFW_INVALID_VALUE: err_code = BasicError::bad_arguments(); break;
				case GLFW_OUT_OF_MEMORY: err_code = BasicError::out_of_memory(); break;
				case GLFW_API_UNAVAILABLE: err_code = BasicError::not_supported(); break;
				case GLFW_VERSION_UNAVAILABLE: err_code = BasicError::not_supported(); break;
				case GLFW_PLATFORM_ERROR: err_code = BasicError::bad_platform_call(); break;
				case GLFW_FORMAT_UNAVAILABLE: err_code = BasicError::not_supported(); break;
				default: err_code = BasicError::bad_platform_call(); break;
				}
				// Handle error.
				if (description)
				{
					set_error(err_code, description);
					return BasicError::error_object();
				}
				return err_code;
			}
			return ErrCode(0);
		}

		inline RV check_glfw_error()
		{
			ErrCode code = encode_glfw_error();
			if (code.code != 0)
			{
				return code;
			}
			return ok;
		}

		inline bool glfw_succeeded()
		{
			return glfwGetError(NULL) == GLFW_NO_ERROR;
		}

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

			// Used to restore window size and pos when exiting from full screen mode.
			u32 m_windowed_width;
			u32 m_windowed_height;
			i32 m_windowed_pos_x;
			i32 m_windowed_pos_y;

            void close();
			bool is_closed() { return m_window == nullptr; }
			bool is_focused() { return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0; }
			RV set_focus() { glfwFocusWindow(m_window); return check_glfw_error(); }
			bool is_minimized() { return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0; }
			bool is_maximized() { return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) != 0; }
			RV set_minimized() { glfwIconifyWindow(m_window); return check_glfw_error(); }
			RV set_maximized() { glfwMaximizeWindow(m_window); return check_glfw_error(); }
			RV set_restored() { glfwRestoreWindow(m_window); return check_glfw_error(); }
			bool is_hovered() { return glfwGetWindowAttrib(m_window, GLFW_HOVERED) != 0; }
			bool is_visible() { return glfwGetWindowAttrib(m_window, GLFW_VISIBLE) != 0; }
			RV set_visible(bool visible) { if (visible) glfwShowWindow(m_window); else glfwHideWindow(m_window); return check_glfw_error(); }
			bool is_resizable() { return glfwGetWindowAttrib(m_window, GLFW_RESIZABLE) != 0; }
			RV set_resizable(bool resizable) { glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE); return check_glfw_error(); }
			bool is_frameless() { return glfwGetWindowAttrib(m_window, GLFW_DECORATED) == 0; }
			RV set_frameless(bool frameless) { glfwSetWindowAttrib(m_window, GLFW_DECORATED, frameless ? GLFW_FALSE : GLFW_TRUE); return check_glfw_error(); }
			Int2U get_position();
			RV set_position(i32 x, i32 y);
			UInt2U get_size();
			RV set_size(u32 width, u32 height);
			UInt2U get_framebuffer_size();
			f32 get_dpi_scale_factor();
			bool is_full_screen() { return get_monitor() != nullptr; }
			monitor_t get_monitor() {return glfwGetWindowMonitor(m_window); }
			RV set_display_settings(const WindowDisplaySettings& display_settings);
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
