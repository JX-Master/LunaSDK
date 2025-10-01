/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventDispatching.hpp
* @author JXMaster
* @date 2025/3/23
*/
#pragma once
#include "Window.hpp"
#include "Display.hpp"

namespace Luna
{
    namespace Window
    {
        //! Dispatches display orientation event.
        //! @details This event is dispatched when the orientation of the display is changed.
        //! @param[in] display The display whose orientation is changed.
        //! @param[in] orientation The new display orientation after change.
        LUNA_WINDOW_API void dispatch_display_orientation_event(display_t display, DisplayOrientation orientation); 

        //! Dispatches display connect event.
        //! @details This event is dispatched when a new display is connected to the platform.
        //! @param[in] display The display that is connected.
        LUNA_WINDOW_API void dispatch_display_connect_event(display_t display);

        //! Dispatches display disconnect event.
        //! @details This event is dispatched when a display is disconnected from the platform.
        //! @param[in] display The display that is disconnected.
        LUNA_WINDOW_API void dispatch_display_disconnect_event(display_t display);

        //! Dispatches display move event.
        //! @details This event is dispatched when the position of the display in desktop coordinates 
        //! is changed. This usually happens when the user changes the display settings in the
        //! system settings.
        //! @param[in] display The display whose position is changed.
        LUNA_WINDOW_API void dispatch_display_move_event(display_t display);

        //! Dispatches window close event.
        //! @details This event is dispatched when one window is requested to be closed, usually
        //! because the user clicks the close button of the window.
        //! @param[in] window The window that is requested to be closed.
        LUNA_WINDOW_API void dispatch_window_close_event(IWindow* window);

        //! Dispatches window focus event.
        //! @details This event is dispatched when one window gains input focus.
        //! @param[in] window The window that gains the input focus.
        LUNA_WINDOW_API void dispatch_window_focus_event(IWindow* window);

        //! Dispatches window lose focus event.
        //! @details This event is dispatched when one window gains input focus.
        //! @param[in] window The window that loses the input focus.
        LUNA_WINDOW_API void dispatch_window_lose_focus_event(IWindow* window);

        //! Dispatches window show event.
        //! @details This event is dispatched when the window's visibility is changed from
        //! hidden to show.
        //! @param[in] window The window whose visibility is changed.
        LUNA_WINDOW_API void dispatch_window_show_event(IWindow* window);

        //! Dispatches window hide event.
        //! @details This event is dispatched when the window's visibility is changed from 
        //! show to hidden.
        //! @param[in] window The window whose visibility is changed.
        LUNA_WINDOW_API void dispatch_window_hide_event(IWindow* window);

        //! Dispatches window resize event.
        //! @details This event is dispatched when the window's size is changed.
        //! @param[in] window The window whose size is changed.
        //! @param[in] width The new width of the window in screen coordinates.
        //! @param[in] height The new height of the window in screen coordinates.
        LUNA_WINDOW_API void dispatch_window_resize_event(IWindow* window, u32 width, u32 height);

        //! Dispatches window framebuffer resize event.
        //! @details This event is dispatched when the window's framebuffer size is changed.
        //! @param[in] window The window whose framebuffer's size is changed.
        //! @param[in] width The new width of the window's framebuffer size in pixels.
        //! @param[in] height The new height of the window's framebuffer size in pixels.
        LUNA_WINDOW_API void dispatch_window_framebuffer_resize_event(IWindow* window, u32 width, u32 height);

        //! Dispatches window move event.
        //! @details This event is dispatched when the window's position is changed.
        //! @param[in] window The window that is moved.
        //! @param[in] x The X position of the window in screen coordinates after move.
        //! @param[in] Y The Y position of the window in screen coordinates after move.
        LUNA_WINDOW_API void dispatch_window_move_event(IWindow* window, i32 x, i32 y);

        //! Dispatches window DPI changed event.
        //! @details This event is dispatched when the window's DPI (dots per inch) is changed. This 
        //! may happen if the user changes the DPI of the display, or moves the window to another display
        //! with different DPI.
        //! @param[in] window The window whose DPI is changed.
        LUNA_WINDOW_API void dispatch_window_dpi_changed_event(IWindow* window);

        //! Dispatches window key down event.
        //! @details This event is dispatched when the user presses one key with one window being focused.
        //! @param[in] window The window that gains keyboard input focus.
        //! @param[in] key The key that is pressed.
        LUNA_WINDOW_API void dispatch_window_key_down_event(IWindow* window, HID::KeyCode key);

        //! Dispatches window key up event.
        //! @details This event is dispatched when the user releases one key with one window being focused.
        //! @param[in] window The window that gains keyboard input focus.
        //! @param[in] key The key that is released.
        LUNA_WINDOW_API void dispatch_window_key_up_event(IWindow* window, HID::KeyCode key);

        //! Dispatches window input character event.
        //! @details The event is dispatched when one input character is sent to the window.
        //! @param[in] window The window that receives the input character.
        //! @param[in] character The Unicode character that is sent to the window.
        LUNA_WINDOW_API void dispatch_window_input_character_event(IWindow* window, c32 character);

        //! Dispatches window mouse enter event.
        //! @details This event is dispatched when the mouse cursor enters the non-covered region of the window.
        //! @param[in] window The window that the mouse cursor is entered.
        LUNA_WINDOW_API void dispatch_window_mouse_enter_event(IWindow* window);

        //! Dispatches window mouse leave event.
        //! @details This event is dispatched when the mouse cursor leaves the non-covered region of the window.
        //! @param[in] window The window that the mouse cursor is leaved.
        LUNA_WINDOW_API void dispatch_window_mouse_leave_event(IWindow* window);

        //! Dispatches window mouse move event.
        //! @details This event is dispatched when the mouse cursor is moved in the non-covered region of the window.
        //! @param[in] window The window that the mouse cursor is moved within.
        //! @param[in] x The new X position of the mouse cursor relative to the window client area.
        //! @param[in] y The new Y position of the mouse cursor relative to the window client area.
        LUNA_WINDOW_API void dispatch_window_mouse_move_event(IWindow* window, i32 x, i32 y);

        //! Dispatches window mouse down event.
        //! @details This event is dispatched when the mouse button is pressed and the target window has mouse input focus.
        //! @param[in] window The window that gains input focus.
        //! @param[in] button The mouse button that is pressed.
        LUNA_WINDOW_API void dispatch_window_mouse_down_event(IWindow* window, HID::MouseButton button);

        //! Dispatches window mouse up event.
        //! @details This event is dispatched when the mouse button is released and the target window has mouse input focus.
        //! @param[in] window The window that gains input focus.
        //! @param[in] button The mouse button that is released.
        LUNA_WINDOW_API void dispatch_window_mouse_up_event(IWindow* window, HID::MouseButton button);

        //! Dispatches window scroll event.
        //! @details This event is dispatched when the window is scrolled by mouse wheel or trackpad and the window gains input
        //! focus.
        //! @param[in] window The window that gains input focus.
        //! @param[in] scroll_x The scroll delta in X dimension.
        //! @param[in] scroll_y The scroll delta in Y dimension.
        LUNA_WINDOW_API void dispatch_window_scroll_event(IWindow* window, f32 scroll_x, f32 scroll_y);

        //! Dispatches window touch down event.
        //! @details This event is emitted when the a new touch point is detected.
        //! @param[in] window The window this event is dispatched to.
        //! @param[in] id The identifier of the touch point. 
        //! This id remains unchanged for the same touch point during different touch events.
        //! @param[in] x The x position of the touch point relative to the window position.
        //! @param[in] y The y position of the touch point relative to the window position.
        LUNA_WINDOW_API void dispatch_window_touch_down_event(IWindow* window, u64 id, f32 x, f32 y);

        //! Dispatches window touch move event.
        //! @details This event is dispatched when the position of one existing touch point is changed.
        //! @param[in] window The window this event is dispatched to.
        //! @param[in] id The identifier of the touch point. 
        //! This id remains unchanged for the same touch point during different touch events.
        //! @param[in] x The x position of the touch point relative to the window position.
        //! @param[in] y The y position of the touch point relative to the window position.
        LUNA_WINDOW_API void dispatch_window_touch_move_event(IWindow* window, u64 id, f32 x, f32 y);

        //! Dispatches window touch up event.
        //! @details This event is dispatched when the a existing touch point is released.
        //! @param[in] window The window this event is dispatched to.
        //! @param[in] id The identifier of the touch point. 
        //! This id remains unchanged for the same touch point during different touch events.
        //! @param[in] x The x position of the touch point relative to the window position.
        //! @param[in] y The y position of the touch point relative to the window position.
        LUNA_WINDOW_API void dispatch_window_touch_up_event(IWindow* window, u64 id, f32 x, f32 y);

        //! Dispatches window drop file event.
        //! @details This eevnt is dispatched when the user drags and drops files into the window.
        //! @param[in] window The window this event is dispatched to.
        //! @param[in] paths The array of paths of files being dropped. Every element of the array 
        //! is one null-terminated string. These strings should be valid before this function returns.
        //! @param[in] num_paths The number of string paths in `paths`.
        LUNA_WINDOW_API void dispatch_window_drop_file_event(IWindow* window, const c8** paths, usize num_paths);
    }
}