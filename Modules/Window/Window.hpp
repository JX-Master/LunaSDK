/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2019/7/10
*/
#pragma once
#include <Runtime/Interface.hpp>
#include <Runtime/Result.hpp>
#include <HID/KeyCode.hpp>
#include <Runtime/Ref.hpp>
#include <Runtime/Span.hpp>
#include <Runtime/Event.hpp>
#include "Monitor.hpp"

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

		//! @interface IWindow
		//! @threadsafe
		//! Represents a system window that can be used to display user interface and can be drawn as surface.
		struct IWindow : virtual Interface
		{
			luiid("{234f4d10-340a-4633-9acc-d70d61f44d23}");

			//! Closes this window.
			//! On single-window platforms, this causes the application to exit.
			virtual void close() = 0;

			//! Whether the window is closed. The window handle is invalid when one window is closed.
			virtual bool is_closed() = 0;

			//! Sets the window as full scrren on the specified monitor.
			//! @param[in] monitor The monitor to set for full scrren.
			virtual RV set_fullscreen(monitor_t monitor, u32 width, u32 height, u32 refresh_rate = 0) = 0;

			//! Restores one window from full screen mode.
			virtual RV unset_fullscreen(i32 x, i32 y, u32 width, u32 height) = 0;

			//! Gets the position of the window client area.
			virtual Int2U get_position() = 0;

			//! Sets the position of the window client area.
			virtual RV set_position(i32 x, i32 y) = 0;

			//! Gets the size of the content area of the window measured in screen coordinates.
			//! @remark The screen coordinates is not necessary measured in pixels. For pixel-related operations,
			//! use `get_framebuffer_size` instead.
			virtual UInt2U get_size() = 0;

			//! Gets the framebuffer size of the window context area in pixels.
			virtual UInt2U get_framebuffer_size() = 0;

			//! Sets the size of the content area of the window measured in screen coordinates.
			virtual RV set_size(u32 width, u32 height) = 0;

			//! Gets the DPI scaling factor, which is the ratio between the current DPI and the platform's default DPI.
			//! @return Returns the DPI scaling factor. The default (unscaled) DPI factor is 1.0.
			//! @remark The DPI scale factor may be used if DPI scaling is enabled on the target monitor.
			virtual f32 get_dpi_scale_factor() = 0;

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
			//! Whether this window is minimizable by clicking the minimize button on the title bar. 
			//! This flag is ignored if the window is borderless.
			minimizable = 0x04,
			//! Whether this window is maximizable by clicking the maximize button on the title bar.
			//! This flag is ignored if the window is borderless.
			maximizable = 0x08,
			//! Lets the system choose a proper initial size for the window.
			default_size = 0x10,
			//! Positions the window at the center of the main screen.
			position_center = 0x20,
			//! Window does not displayed when being created.
			hidden = 0x40,
		};

		//! Creates a new window. The window will be displayed unless WindowCreationFlag::hidden is set.
		//! @param[in] title The title of the window.
		//! @param[in] x The x position of the window in screen space. If `WindowCreationFlag::default_size_pos` is set, this parameter is ignored.
		//! @param[in] y The y position of the window in screen space. If `WindowCreationFlag::default_size_pos` is set, this parameter is ignored.
		//! @param[in] width The width of the window. If `WindowCreationFlag::default_size_pos` is set, this parameter is ignored.
		//! @param[in] height The height of the window. If `WindowCreationFlag::default_size_pos` is set, this parameter is ignored.
		//! @param[in] monitor The monitor to use for full screen mode, or `nullptr` for windowed mode.
		//! @param[in] flags Additional window creation flags.
		//! @return Returns the window object, returns `BasicError::not_supportted` on single-window platforms.
		//! @remark This function must only be called by the main thread.
		LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, i32 x, i32 y,
			i32 width, i32 height, 
			monitor_t monitor = nullptr,
			WindowCreationFlag flags = WindowCreationFlag::none);

		//! Gets the application main window.
		//! On single-window systems (like mobile phones, tablets, consoles), the main window is created by the system
		//! and can be fetched by this call; on multiple-window systems, the main window is created by the user and 
		//! set by `set_main_window`.
		//! @return Returns the main window. Note that the returned window may be `nullptr` if the main window is not present
		//! on multi-window platforms.
		LUNA_WINDOW_API IWindow* get_main_window();

		//! Sets the main window.
		//! This call is not thread-safe and should be called when the application initializes. For single-window systems,
		//! the main window is set up for you, so you don't need to call this function.
		LUNA_WINDOW_API RV set_main_window(IWindow* window);
	}
}
