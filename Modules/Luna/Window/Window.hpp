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
        //! @addtogroup Window Window
        //! The Window module provides window management functionalities of the underlying system.
        //! @{
        
        //! Identifies keys that are pressed in window mouse events.
        enum class ModifierKeyFlag : u8
        {
            none = 0x00,
            //! Ctrl key.
            ctrl = 0x01,
            //! Alt key.
            menu = 0x02,
            //! Shift key.
            shift = 0x04,
            //! Windows key on Windows, Command key on Mac.
            system = 0x08,
        };

        struct IWindow;

        //! The hanlder for window close event.
        using window_close_event_handler_t = void(IWindow* window);

        //! The handler for window focus event.
        using window_focus_event_handler_t = void(IWindow* window);

        //! The handler for window lose foucs event.
        using window_lose_focus_event_handler_t = void(IWindow* window);

        //! The handler for window show event.
        using window_show_event_handler_t = void(IWindow* window);

        //! The handler for window hide event.
        using window_hide_event_handler_t = void(IWindow* window);

        //! The handler for window resize event.
        using window_resize_event_handler_t = void(IWindow* window, u32 width, u32 height);

        //! The handler for window framebuffer resize event.
        using window_framebuffer_resize_event_handler_t = void(IWindow* window, u32 width, u32 height);

        //! The handler for window move resize event.
        using window_move_event_handler_t = void(IWindow* window, i32 x, i32 y);

        //! The handler for window begin resize move event.
        using window_begin_resize_move_t = void(IWindow* window);

        //! The handler for window end resize move event.
        using window_end_resize_move_t = void(IWindow* window);

        //! The handler for window dpi changed event.
        using window_dpi_changed_event_handler_t = void(IWindow* window, f32 dpi_scale);

        //! The handler for window key down event.
        using window_key_down_event_handler_t = void(IWindow* window, HID::KeyCode key);

        //! The handler for window key up event.
        using window_key_up_event_handler_t = void(IWindow* window, HID::KeyCode key);

        //! The handler for window input character event.
        using window_input_character_event_handler_t = void(IWindow* window, c32 character);

        //! The handler for mouse move event.
        using window_mouse_move_event_handler_t = void(IWindow* window, i32 x, i32 y);

        //! The handler for mouse down event.
        using window_mouse_down_event_handler_t = void(IWindow* window, ModifierKeyFlag modifier_flags, HID::MouseButton button);

        //! The handler for mouse up event.
        using window_mouse_up_event_handler_t = void(IWindow* window, ModifierKeyFlag modifier_flags, HID::MouseButton button);
        
        //! The handler for mouse up event.
        using window_mouse_wheel_event_handler_t = void(IWindow* window, f32 x_wheel_delta, f32 y_wheel_delta);

        //! Identifies one touch point in one window touch event.
        struct WindowEventTouchPoint
        {
            //! The unique ID that idenfity every touch point between touch events.
            u32 id;
            //! The position of the touch point, relative to the touch window.
            Int2U position;
        };

        //! The handler for touch event.
        //! @param[in] touches The span that includes all touch points for this event.
        //! @param[in] changed_mask A bit-combined mask to identify whether every touch point is changed between multiple touch 
        //! events. Use `(changed_mask & (1 << i))` to test the touch point `i`, where `i` is the index of `touches`.
        using window_touch_event_handler_t = void(IWindow* window, Span<WindowEventTouchPoint> touches, u32 changed_mask);

        //! The handler for drop file event.
        using window_drop_file_event_handler_t = void(IWindow* window, i32 x, i32 y, Span<const c8*> paths);

        //! Specify this value as `x` or `y` of the window to let windowing system choose one suitable position for the window.
        constexpr i32 DEFAULT_POS = I32_MAX;

        //! The window display settings.
        struct WindowDisplaySettings
        {
            //! The monitor that displays the window. 
            //! 
            //! On full screen mode, if this is `nullptr`, window will be displayed on the main monitor.
            //! 
            //! This must be `nullptr` if `full_screen` is `false`.
            monitor_t monitor;
            //! The X position of the window.
            //! 
            //! If @ref DEFAULT_POS is specified, the system will choose the most suitable position for the window.
            //! 
            //! This must be set to @ref DEFAULT_POS if `full_screen` is `true`.
            i32 x;
            //! The y position of the window.
            //! 
            //! If @ref DEFAULT_POS is specified, the system will choose the most suitable position for the window.
            //! 
            //! This must be set to @ref DEFAULT_POS if `full_screen` is `true`.
            i32 y;
            //! The width of the window.
            //! 
            //! If `0` is specified, the system will choose the suitable size for the window.
            u32 width;
            //! The height of the window.
            //! 
            //! If `0` is specified, the system will choose the suitable size for the window.
            u32 height;
            //! The refresh rate of the window.
            //! 
            //! If `0` is specified, the system will choose the suitable refresh rate for the window.
            //! 
            //! This must be set to `0` if `full_screen` is `false`.
            u32 refresh_rate;
            //! Whether this window is full screen.
            bool full_screen;

            //! Creates one window display settings structure that specifies one windowed window.
            //! @param[in] x The X position of the window. If @ref DEFAULT_POS is specified, the system will choose the suitable position for the window.
            //! @param[in] y The X position of the window. If @ref DEFAULT_POS is specified, the system will choose the suitable position for the window.
            //! @param[in] width The width of the window. If `0` is specified, the system will choose the suitable size for the window.
            //! @param[in] height The height of the window. If `0` is specified, the system will choose the suitable size for the window.
            //! @return Returns the created structure.
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
            //! Creates one window display settings structure that specifies one full screen window.
            //! @param[in] monitor The monitor that displays the window. If this is `nullptr`, window will be displayed on the main monitor.
            //! @param[in] width The width of the window. If `0` is specified, the system will choose the suitable size for the window.
            //! @param[in] height The height of the window. If `0` is specified, the system will choose the suitable size for the window.
            //! @param[in] refresh_rate The refresh rate of the window. If `0` is specified, the system will choose the suitable refresh rate for the window.
            //! @return Returns the created structure.
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
        //! Represents a system window that can be used to display user interface and can be drawn as surface.
        struct IWindow : virtual Interface
        {
            luiid("{234f4d10-340a-4633-9acc-d70d61f44d23}");

            //! Closes this window.
            //! On single-window platforms, this causes the application to exit.
            virtual void close() = 0;

            //! Checks whether the window is closed. The window handle is invalid when one window is closed.
            //! @return Returns `true` if the window is closed. Returns `false` otherwise.
            virtual bool is_closed() = 0;

            //! Checks whether the window has input focus.
            //! @return Returns `true` if the window is focused. Returns `false` otherwise.
            virtual bool is_focused() = 0;

            //! Brings this window to front and acquires input focus for the window.
            virtual RV set_focus() = 0;

            //! Checks whether the window is minimized.
            //! @return Returns `true` if the window is minimized. Returns `false` otherwise.
            virtual bool is_minimized() = 0;

            //! Checks whether the window is maximized.
            //! @return Returns `true` if the window is maximized. Returns `false` otherwise.
            virtual bool is_maximized() = 0;

            //! Minimize the window.
            virtual RV set_minimized() = 0;

            //! Maximize the window.
            virtual RV set_maximized() = 0;

            //! Restore the window from minimized or maximized state.
            virtual RV set_restored() = 0;

            //! Checks whether the cursor is currently directly over the content area of the window.
            //! @return Returns `true` if the window is hovered. Returns `false` otherwise.
            virtual bool is_hovered() = 0;

            //! Checks whether the window is visible.
            //! @return Returns `true` if the window is visible. Returns `false` otherwise.
            virtual bool is_visible() = 0;

            //! Sets the visibility of the window.
            //! @param[in] visible The new visibility of the window (`true` for visible).
            virtual RV set_visible(bool visible) = 0;

            //! Checks whether the window is resizable by dragging the border of the window.
            //! @return Returns `true` if the window is resizable. Returns `false` otherwise.
            virtual bool is_resizable() = 0;

            //! Sets the resizable state of the window.
            //! @param[in] resizable The new resizable state of the window (`true` for resizable).
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
            //! @param[in] title The title to set.
            //! @par Valid Usage
            //! * `title` must be one null-terminated UTF-8 string.
            virtual RV set_title(const c8* title) = 0;

            //! Sets the window display settings.
            //! @param[in] display_settings The new window display settings.
            virtual RV set_display_settings(const WindowDisplaySettings& display_settings) = 0;

            //! Converts one screen coordinate to one client coordinate.
            virtual Int2U screen_to_client(const Int2U& point) = 0;

            //! Converts one client coordinate to one screen coordinate.
            virtual Int2U client_to_screen(const Int2U& point) = 0;

            //! Gets the close event of this window. 
            //! @details Usually the user should close the window by calling @ref IWindow::close to respond this event.
            //! This event will be emitted when the close button of the window is pressed.
            //! @return Returns one reference of the event object.
            virtual Event<window_close_event_handler_t>& get_close_event() = 0;
            //! Gets the focus event of this window.
            //! @details This event will be emitted when the window gains focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_focus_event_handler_t>& get_focus_event() = 0;
            //! Gets the lose focus event of this window.
            //! This event will be emitted when the window loses focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_lose_focus_event_handler_t>& get_lose_focus_event() = 0;
            //! Gets the show event of this window.
            //! @details This event will be emitted when the window is visible to the user.
            //! The client code should continue receiving inputs from the window
            //! and continue rendering to the window after receiving this event.
            //! @return Returns one reference of the event object.
            virtual Event<window_show_event_handler_t>& get_show_event() = 0;
            //! Gets the hide event of this window.
            //! @details This event will be emitted when the window is completly hidden from the user.
            //! The client code should stop receiving inputs from the window 
            //! and stop rendering to the window after receiving this event.
            //! @return Returns one reference of the event object.
            virtual Event<window_hide_event_handler_t>& get_hide_event() = 0;
            //! Gets the resize event of this window.
            //! @details This event will be emitted when the window size is changed.
            //! The new size may be 0 if the window is minimized.
            //! @return Returns one reference of the event object.
            virtual Event<window_resize_event_handler_t>& get_resize_event() = 0;
            //! Gets the framebuffer resize event of this window.
            //! @details This event will be emitted when the window's framebuffer size is changed.
            //! @return Returns one reference of the event object.
            virtual Event<window_framebuffer_resize_event_handler_t>& get_framebuffer_resize_event() = 0;
            //! Gets the move event of this window.
            //! @details This event will be emitted when the window position is changed.
            //! @return Returns one reference of the event object.
            virtual Event<window_move_event_handler_t>& get_move_event() = 0;
            //! Gets the begin resize move event of this window.
            //! @details This event will be emitted when the user starts to change the window rect.
            //! @return Returns one reference of the event object.
            virtual Event<window_begin_resize_move_t>& get_begin_resize_move_event() = 0;
            //! Gets the end resize move event of this window.
            //! @details This event will be emitted when the user finishes changing the window rect.
            //! @return Returns one reference of the event object.
            virtual Event<window_end_resize_move_t>& get_end_resize_move_event() = 0;
            //! Gets the end dpi changed event of this window.
            //! @details This event will be emitted when the window DPI is changed. This may happen when the window is moved to another minitor
            //! with different DPI settings.
            //! @return Returns one reference of the event object.
            virtual Event<window_dpi_changed_event_handler_t>& get_dpi_changed_event() = 0;
            //! Gets the key down event of this window.
            //! @details This event will be emitted when one keyboard key is pressed and the window has key focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_key_down_event_handler_t>& get_key_down_event() = 0;
            //! Gets the key up event of this window.
            //! @details This event will be emitted when one keyboard key is released and the window has key focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_key_up_event_handler_t>& get_key_up_event() = 0;
            //! Gets the input character event of this window.
            //! @details This event will be emitted when one character input is transmitted to this window.
            //! @return Returns one reference of the event object.
            virtual Event<window_input_character_event_handler_t>& get_input_character_event() = 0;
            //! Gets the mouse move event of this window.
            //! @details This event will be emitted when the mouse is moved and the window has mouse focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_mouse_move_event_handler_t>& get_mouse_move_event() = 0;
            //! Gets the mouse down event of this window.
            //! @details This event will be emitted when one mouse button is pressed and the window has mouse focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_mouse_down_event_handler_t>& get_mouse_down_event() = 0;
            //! Gets the mouse up event of this window.
            //! @details This event will be emitted when one mouse button is released and the window has mouse focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_mouse_up_event_handler_t>& get_mouse_up_event() = 0;
            //! Gets the mouse wheel event of this window.
            //! @details This event will be emitted when the mouse wheel is scrolled and the window has mouse focus.
            //! @return Returns one reference of the event object.
            virtual Event<window_mouse_wheel_event_handler_t>& get_mouse_wheel_event() = 0;
            //! Gets the touch event of this window.
            //! @details This event will be emitted when the window is focused and touched.
            //! @return Returns one reference of the event object.
            virtual Event<window_touch_event_handler_t>& get_touch_event() = 0;
            //! Gets the drop file event of this window.
            //! @details This event will be emitted when one file is dropped on the window.
            //! @return Returns one reference of the event object.
            virtual Event<window_drop_file_event_handler_t>& get_drop_file_event() = 0; 
        };

        //! Processes window events for all windows created from the current thread.
        //! @param[in] wait_events Whether to block the current thread until there are at least one event received.
        LUNA_WINDOW_API void poll_events(bool wait_events = false);

        //! Flags that specifies the initial state and style of the window.
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

        //! Creates one new window. The new window will be displayed immediately unless @ref WindowCreationFlag::hidden is set.
        //! @param[in] title The title of the window.
        //! @param[in] display_settings The initial display settings for the window.
        //! @param[in] flags Additional window creation flags.
        //! @return Returns the created window.
        //! @par Valid Usage
        //! * This function can only be called by the main thread.
        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, 
            const WindowDisplaySettings& display_settings,
            WindowCreationFlag flags = WindowCreationFlag::none);
        
        //! @}
    }

    LUNA_WINDOW_API Module* module_window();
}