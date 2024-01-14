/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2019/7/10
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/HID/KeyCode.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Event.hpp>
#include "Monitor.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
	namespace Window
	{
		enum class ModifierKeyFlag : u8
		{
			none = 0x00,
			ctrl = 0x01,
			menu = 0x02,
			shift = 0x04,
			system = 0x08,	// Windows key on Windows, Command key on Mac.
		};

		struct IWindow;

		//! Emitted when the close button of the window is pressed.
		//! Usually the user should close the window by calling `IWindow::close` to respond this event.
		using window_close_event_handler_t = void(IWindow* window);

		//! Emitted when the window gains focus.
		using window_focus_event_handler_t = void(IWindow* window);

		//! Emitted when the window loses focus.
		using window_lose_focus_event_handler_t = void(IWindow* window);

		//! Emitted when the window is visible to the user.
		//! The client code should continue receiving inputs from the window
		//! and continue rendering to the window after receiving this event.
		using window_show_event_handler_t = void(IWindow* window);

		//! Emitted when the window is completly hidden from the user. 
		//! The client code should stop receiving inputs from the window 
		//! and stop rendering to the window after receiving this event.
		using window_hide_event_handler_t = void(IWindow* window);

		//! Emitted when the window size is changed.
		//! The new size may be 0 if the window is minimized.
		using window_resize_event_handler_t = void(IWindow* window, u32 width, u32 height);

		//! Emitted when the window's framebuffer size is changed.
		using window_framebuffer_resize_event_handler_t = void(IWindow* window, u32 width, u32 height);

		//! Emitted when the window position is changed.
		using window_move_event_handler_t = void(IWindow* window, i32 x, i32 y);

		//! Emitted when the user starts to change the window rect.
		using window_begin_resize_move_t = void(IWindow* window);

		//! Emitted when the user finishes changing the window rect.
		using window_end_resize_move_t = void(IWindow* window);

		//! Emitted when the window DPI is changed. This may happen when the window is moved to another minitor
		//! with different DPI settings.
		using window_dpi_changed_event_handler_t = void(IWindow* window, f32 dpi_scale);

		//! Emitted when one keyboard key is pressed and the window has key focus.
		using window_key_down_event_handler_t = void(IWindow* window, HID::KeyCode key);

		//! Emitted when one keyboard key is released and the window has key focus.
		using window_key_up_event_handler_t = void(IWindow* window, HID::KeyCode key);

		//! Emitted when one character input is transmitted to this window.
		using window_input_character_event_handler_t = void(IWindow* window, c32 character);

		//! Emitted when the mouse is moved and the window has mouse focus.
		using window_mouse_move_event_handler_t = void(IWindow* window, i32 x, i32 y);

		//! Emitted when one mouse button is pressed and the window has mouse focus.
		using window_mouse_down_event_handler_t = void(IWindow* window, ModifierKeyFlag modifier_flags, HID::MouseButton button);

		//! Emitted when one mouse button is released and the window has mouse focus.
		using window_mouse_up_event_handler_t = void(IWindow* window, ModifierKeyFlag modifier_flags, HID::MouseButton button);
		
		//! Emitted when the mouse wheel is scrolled and the window has mouse focus.
		using window_mouse_wheel_event_handler_t = void(IWindow* window, f32 x_wheel_delta, f32 y_wheel_delta);

		struct WindowEventTouchPoint
		{
			//! The unique ID that idenfity every touch point between touch events.
			u32 id;
			//! The position of the touch point, relative to the touch window.
			Int2U position;
		};

		//! Emitted when the window is focused and touched.
		//! @param[in] touches The span that includes all touch points for this event.
		//! @param[in] changed_mask A bit-combined mask to identify whether every touch point is changed between multiple touch 
		//! events. Use `(changed_mask & (1 << i))` to test the touch point `i`, where `i` is the index of `touches`.
		using window_touch_event_handler_t = void(IWindow* window, Span<WindowEventTouchPoint> touches, u32 changed_mask);

		//! Emitted when one file is dropped on the window.
		using window_drop_file_event_handler_t = void(IWindow* window, i32 x, i32 y, Span<const c8*> paths);

		enum class WindowFlag : u32
		{
			none = 0x00,
			//! Whether this window is borderless. One borderless window does not have border, titlebar 
			//! and close/minimize/maximize buttons.
			frameless = 0x01,
			//! Whether this window is resizable by dragging the border of the window.
			resizable = 0x02,
			//! Whether this window is minimizable by clicking the minimize button on the title bar.
			minimizable = 0x04,
			//! Whether this window is maximizable by clicking the maximize button on the title bar.
			maximizable = 0x08,
		};

		//! Specify this value as `x` or `y` of the window to let windowing system choose one suitable position for the window.
		constexpr i32 DEFAULT_POS = I32_MAX;

		struct WindowDisplaySettings
		{
			monitor_t monitor;
			i32 x;
			i32 y;
			u32 width;
			u32 height;
			u32 refresh_rate;
			bool full_screen;

			static WindowDisplaySettings as_windowed(i32 x = DEFAULT_POS, i32 y = DEFAULT_POS, u32 width = 0, u32 height = 0)
			{
				WindowDisplaySettings ret;
				ret.monitor = nullptr;
				ret.x = x;
				ret.y = y;
				ret.width = width;
				ret.height = height;
				ret.refresh_rate = 0;
				ret.full_screen = false;
				return ret;
			}
			static WindowDisplaySettings as_full_screen(monitor_t monitor = nullptr, u32 width = 0, u32 height = 0, u32 refresh_rate = 0)
			{
				WindowDisplaySettings ret;
				ret.monitor = monitor;
				ret.x = DEFAULT_POS;
				ret.y = DEFAULT_POS;
				ret.width = width;
				ret.height = height;
				ret.refresh_rate = refresh_rate;
				ret.full_screen = true;
				return ret;
			}
		};

		//! @interface IWindow
		//! @threadsafe
		//! Represents a system window that can be used to display user interface and can be drawn as surface.
		struct IWindow : virtual Interface
		{
			luiid("{234f4d10-340a-4633-9acc-d70d61f44d23}");

			//! Closes this window.
			//! On single-window platforms, this causes the application to exit.
			virtual void close() = 0;

			//! Checks whether the window is closed. The window handle is invalid when one window is closed.
			virtual bool is_closed() = 0;

			//! Checks whether the window has input focus.
			virtual bool is_focused() = 0;

			//! Brings this window to front and acquires input focus for the window.
			virtual RV set_focus() = 0;

			//! Checks whether the window is minimized.
			virtual bool is_minimized() = 0;

			//! Checks whether the window is maximized.
			virtual bool is_maximized() = 0;

			//! Minimize the window.
			virtual RV set_minimized() = 0;

			//! Maximize the window.
			virtual RV set_maximized() = 0;

			//! Restore the window from minimized or maximized state.
			virtual RV set_restored() = 0;

			//! Checks whether the cursor is currently directly over the content area of the window.
			virtual bool is_hovered() = 0;

			//! Checks whether the window is visible.
			virtual bool is_visible() = 0;

			//! Sets the visibility of the window.
			virtual RV set_visible(bool visible) = 0;

			//! Checks whether the window is resizable by dragging the border of the window.
			virtual bool is_resizable() = 0;

			//! Sets the resizable state of the window.
			virtual RV set_resizable(bool resizable) = 0;

			//! Checks whether the window is frameless.
			//! One frameless window does not have border, titlebar 
			//! and close/minimize/maximize buttons.
			virtual bool is_frameless() = 0;

			//! Sets the frameless state of the window.
			virtual RV set_frameless(bool frameless) = 0;

			//! Gets the position of the window client area.
			virtual Int2U get_position() = 0;

			//! Sets the position of the window client area.
			virtual RV set_position(i32 x, i32 y) = 0;

			//! Gets the size of the content area of the window measured in screen coordinates.
			//! @remark The screen coordinates is not necessary measured in pixels. For pixel-related operations,
			//! use `get_framebuffer_size` instead.
			virtual UInt2U get_size() = 0;

			//! Sets the size of the content area of the window measured in screen coordinates.
			virtual RV set_size(u32 width, u32 height) = 0;

			//! Gets the framebuffer size of the window context area in pixels.
			virtual UInt2U get_framebuffer_size() = 0;

			//! Gets the DPI scaling factor, which is the ratio between the current DPI and the platform's default DPI.
			//! @return Returns the DPI scaling factor. The default (unscaled) DPI factor is 1.0.
			//! @remark The DPI scale factor may be used if DPI scaling is enabled on the target monitor.
			virtual f32 get_dpi_scale_factor() = 0;

			//! Checks whether the window is full screen.
			virtual bool is_full_screen() = 0;

			//! Gets the monitor that one full screen window is attached to.
			//! Returns `nullptr` if the window is not in full-screen mode.
			virtual monitor_t get_monitor() = 0;

			//! Sets the window title.
			virtual RV set_title(const c8* title) = 0;

			//! Sets the window display settings.
			virtual RV set_display_settings(const WindowDisplaySettings& display_settings) = 0;

			//! Converts one screen coordinate to one client coordinate.
			virtual Int2U screen_to_client(const Int2U& point) = 0;

			//! Converts one client coordinate to one screen coordinate.
			virtual Int2U client_to_screen(const Int2U& point) = 0;

			virtual Event<window_close_event_handler_t>& get_close_event() = 0;
			virtual Event<window_focus_event_handler_t>& get_focus_event() = 0;
			virtual Event<window_lose_focus_event_handler_t>& get_lose_focus_event() = 0;
			virtual Event<window_show_event_handler_t>& get_show_event() = 0;
			virtual Event<window_hide_event_handler_t>& get_hide_event() = 0;
			virtual Event<window_resize_event_handler_t>& get_resize_event() = 0;
			virtual Event<window_framebuffer_resize_event_handler_t>& get_framebuffer_resize_event() = 0;
			virtual Event<window_move_event_handler_t>& get_move_event() = 0;
			virtual Event<window_begin_resize_move_t>& get_begin_resize_move_event() = 0;
			virtual Event<window_end_resize_move_t>& get_end_resize_move_event() = 0;
			virtual Event<window_dpi_changed_event_handler_t>& get_dpi_changed_event() = 0;
			virtual Event<window_key_down_event_handler_t>& get_key_down_event() = 0;
			virtual Event<window_key_up_event_handler_t>& get_key_up_event() = 0;
			virtual Event<window_input_character_event_handler_t>& get_input_character_event() = 0;
			virtual Event<window_mouse_move_event_handler_t>& get_mouse_move_event() = 0;
			virtual Event<window_mouse_down_event_handler_t>& get_mouse_down_event() = 0;
			virtual Event<window_mouse_up_event_handler_t>& get_mouse_up_event() = 0;
			virtual Event<window_mouse_wheel_event_handler_t>& get_mouse_wheel_event() = 0;
			virtual Event<window_touch_event_handler_t>& get_touch_event() = 0;
			virtual Event<window_drop_file_event_handler_t>& get_drop_file_event() = 0;
		};

		//! Processes window events for all windows created from the current thread.
		//! @param[in] wait_events Whether to block the current thread until there are at least one event received.
		LUNA_WINDOW_API void poll_events(bool wait_events = false);

		enum class WindowCreationFlag : u32
		{
			none = 0x00,
			//! Whether this window is borderless. One borderless window does not have border, titlebar 
			//! and close/minimize/maximize buttons.
			borderless = 0x01,
			//! Whether this window is resizable by dragging the border of the window.
			//! This flag is effective in normal and borderless mode.
			resizable = 0x02,
			//! Window does not displayed when being created.
			hidden = 0x04,
		};

		//! Creates a new window. The window will be displayed unless WindowCreationFlag::hidden is set.
		//! @param[in] title The title of the window.
		//! @param[in] display_settings The initial display settings for the window.
		//! @param[in] flags Additional window creation flags.
		//! @return Returns the window object, returns `BasicError::not_supportted` on single-window platforms.
		//! @remark This function must only be called by the main thread.
		LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, 
			const WindowDisplaySettings& display_settings,
			WindowCreationFlag flags = WindowCreationFlag::none);
	}

	LUNA_WINDOW_API Module* module_window();
}