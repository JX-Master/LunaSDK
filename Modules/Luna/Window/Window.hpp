/*!
* This file is a portion of LunaSDK.
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
#include "Display.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window Window
        //! The Window module provides window management functionalities of the underlying system.
        //! @{

        struct IWindow;

        //! A set of events that can be monitored by the application for the specified window.
        struct WindowEvents
        {
            //! Dispatched when one window is requested to be closed, usually
            //! because the user clicks the close button of the window.
            //! @param[in] window The window that is requested to be closed.
            Event<void(IWindow* window)> close;
            //! Dispatched when one window is destroyed. The handler should clean up any resource 
            //! attached to this window.
            //! @param[in] window The window that is destroyed.
            Event<void(IWindow* window)> destroy;
            //! Dispatched when one window gains input focus.
            //! @param[in] window The window that gains the input focus.
            Event<void(IWindow* window)> input_focus;
            //! Dispatched when one window gains input focus.
            //! @param[in] window The window that loses the input focus.
            Event<void(IWindow* window)> lose_input_focus;
            //! Dispatched when the window's visibility is changed from
            //! hidden to show.
            //! @param[in] window The window whose visibility is changed.
            Event<void(IWindow* window)> show;
            //! Dispatched when the window's visibility is changed from 
            //! show to hidden.
            //! @param[in] window The window whose visibility is changed.
            Event<void(IWindow* window)> hide;
            //! Dispatched when the window's size is changed.
            //! @param[in] window The window whose size is changed.
            //! @param[in] width The new width of the window in screen coordinates.
            //! @param[in] height The new height of the window in screen coordinates.
            Event<void(IWindow* window, u32 width, u32 height)> resize;
            //! Dispatched when the window's framebuffer size is changed.
            //! @param[in] window The window whose framebuffer's size is changed.
            //! @param[in] width The new width of the window's framebuffer size in pixels.
            //! @param[in] height The new height of the window's framebuffer size in pixels.
            Event<void(IWindow* window, u32 width, u32 height)> framebuffer_resize;
            //! Dispatched when when the window's position is changed.
            //! @param[in] window The window that is moved.
            //! @param[in] x The X position of the window in screen coordinates after move.
            //! @param[in] Y The Y position of the window in screen coordinates after move.
            Event<void(IWindow* window, i32 x, i32 y)> move;
            //! Dispatched when when the window's DPI (dots per inch) is changed. This 
            //! may happen if the user changes the DPI of the display, or moves the window to another display
            //! with different DPI.
            //! @param[in] window The window whose DPI is changed.
            Event<void(IWindow* window)> dpi_scale_changed;
            //! Dispatched when the user presses one key with one window being focused.
            //! @param[in] window The window that gains keyboard input focus.
            //! @param[in] key The key that is pressed.
            Event<void(IWindow* window, HID::KeyCode key)> key_down;
            //! Dispatched when the user releases one key with one window being focused.
            //! @param[in] window The window that gains keyboard input focus.
            //! @param[in] key The key that is released.
            Event<void(IWindow* window, HID::KeyCode key)> key_up;
            //! Dispatched when the windowr receives input text.
            //! @param[in] window The window that receives the input character.
            //! @param[in] text The input text in UTF-8 encoding. The text is null-terminated.
            //! @param[in] length The length of the input text.
            Event<void(IWindow* window, const c8* text, usize length)> input_text;
            //! Dispatched when the mouse cursor enters the non-covered region of the window.
            //! @param[in] window The window that the mouse cursor is entered.
            Event<void(IWindow* window)> mouse_enter;
            //! Dispatched when the mouse cursor leaves the non-covered region of the window.
            //! @param[in] window The window that the mouse cursor is leaved.
            Event<void(IWindow* window)> mouse_leave;
            //! Dispatched when the mouse cursor is moved in the non-covered region of the window.
            //! @param[in] window The window that the mouse cursor is moved within.
            //! @param[in] x The new X position of the mouse cursor relative to the window client area.
            //! @param[in] y The new Y position of the mouse cursor relative to the window client area.
            Event<void(IWindow* window, i32 x, i32 y)> mouse_move;
            //! Dispatched when the mouse button is pressed and the target window has mouse input focus.
            //! @param[in] window The window that gains input focus.
            //! @param[in] button The mouse button that is pressed.
            Event<void(IWindow* window, HID::MouseButton button)> mouse_down;
            //! Dispatched when the mouse button is released and the target window has mouse input focus.
            //! @param[in] window The window that gains input focus.
            //! @param[in] button The mouse button that is released.
            Event<void(IWindow* window, HID::MouseButton button)> mouse_up;
            //! Dispatched when the window is scrolled by mouse wheel or trackpad and the window gains input
            //! focus.
            //! @param[in] window The window that gains input focus.
            //! @param[in] scroll_x The scroll delta in X dimension.
            //! @param[in] scroll_y The scroll delta in Y dimension.
            Event<void(IWindow* window, f32 scroll_x, f32 scroll_y)> scroll;
            //! Dispatched when the a new touch point is detected.
            //! @param[in] window The window this event is dispatched to.
            //! @param[in] id The identifier of the touch point. 
            //! This id remains unchanged for the same touch point during different touch events.
            //! @param[in] x The x position of the touch point relative to the window position.
            //! @param[in] y The y position of the touch point relative to the window position.
            Event<void(IWindow* window, u64 id, f32 x, f32 y)> touch_down;
            //! Dispatched when the position of one existing touch point is changed.
            //! @param[in] window The window this event is dispatched to.
            //! @param[in] id The identifier of the touch point. 
            //! This id remains unchanged for the same touch point during different touch events.
            //! @param[in] x The x position of the touch point relative to the window position.
            //! @param[in] y The y position of the touch point relative to the window position.
            Event<void(IWindow* window, u64 id, f32 x, f32 y)> touch_move;
            //! Dispatched when the a existing touch point is released.
            //! @param[in] window The window this event is dispatched to.
            //! @param[in] id The identifier of the touch point. 
            //! This id remains unchanged for the same touch point during different touch events.
            //! @param[in] x The x position of the touch point relative to the window position.
            //! @param[in] y The y position of the touch point relative to the window position.
            Event<void(IWindow* window, u64 id, f32 x, f32 y)> touch_up;
            //! Dispatched when the user drags and drops files into the window.
            //! @param[in] window The window this event is dispatched to.
            //! @param[in] paths The array of paths of files being dropped. Every element of the array 
            //! is one null-terminated string. These strings should be valid before this function returns.
            //! @param[in] num_paths The number of string paths in `paths`.
            //! @param[in] x The x position of the drop point relative to the window position.
            //! @param[in] y The x position of the drop point relative to the window position.
            Event<void(IWindow* window, const c8** paths, usize num_paths, f32 x, f32 y)> drop_file;

            void reset()
            {
                close.clear();
                destroy.clear();
                input_focus.clear();
                lose_input_focus.clear();
                show.clear();
                hide.clear();
                resize.clear();
                framebuffer_resize.clear();
                move.clear();
                dpi_scale_changed.clear();
                key_down.clear();
                key_up.clear();
                input_text.clear();
                mouse_enter.clear();
                mouse_leave.clear();
                mouse_move.clear();
                mouse_down.clear();
                mouse_up.clear();
                scroll.clear();
                touch_move.clear();
                touch_down.clear();
                touch_up.clear();
                drop_file.clear();
            }
        };

        //! Specify this value as `x` or `y` of the window to let windowing system choose one suitable position for the window.
        constexpr i32 DEFAULT_POS = I32_MAX;

        enum class WindowStyleFlag : u32
        {
            none = 0x00,
            //! Whether this window is resizable by dragging the border of the window.
            resizable = 0x01,
            //! Disables all decorations for the window.
            //! If this is set, `resizable` has no effect.
            borderless = 0x02,
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
            //! @return Returns `true` if the window has input focus. Returns `false` otherwise.
            virtual bool has_input_focus() = 0;

            //! Checks whether the window has mouse focus.
            //! @return Returns `true` if the window has mouse focus. Returns `false` otherwise.
            virtual bool has_mouse_focus() = 0;

            //! Brings this window to front and acquires input focus for the window.
            virtual RV set_foreground() = 0;

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

            virtual WindowStyleFlag get_style() = 0;

            virtual RV set_style(WindowStyleFlag style) = 0;

            //! Gets the position of the window client area in screen coordinates.
            //! @return Returns the position of the window client area in screen coordinates.
            virtual Int2U get_position() = 0;

            //! Sets the position of the window client area.
            //! @param[in] x The new X (left) position of the window client area in screen coordinates.
            //! @param[in] y The new Y (top) position of the window client area in screen coordinates.
            virtual RV set_position(i32 x, i32 y) = 0;

            //! Gets the size of the content area of the window measured in screen coordinates.
            //! @return Returns the size of the content area of the window measured in screen coordinates.
            //! @remark The screen coordinates is not necessary measured in pixels. For pixel-related operations,
            //! use `get_framebuffer_size` instead.
            virtual UInt2U get_size() = 0;

            //! Sets the size of the content area of the window measured in screen coordinates.
            //! @param[in] width The width of the content area of the window measured in screen coordinates.
            //! @param[in] height The height of the content area of the window measured in screen coordinates.
            virtual RV set_size(u32 width, u32 height) = 0;

            //! Gets the framebuffer size of the window context area in pixels.
            //! @return Returns the framebuffer size of the window context area in pixels.
            virtual UInt2U get_framebuffer_size() = 0;

            //! Gets the DPI scaling factor, which is the ratio between the current DPI and the platform's default DPI.
            //! @return Returns the DPI scaling factor. The default (unscaled) DPI factor is 1.0.
            //! @remark The DPI scale factor may be used if DPI scaling is enabled on the target display.
            virtual f32 get_dpi_scale_factor() = 0;

            //! Sets the window title.
            //! @param[in] title The title to set.
            //! @par Valid Usage
            //! * `title` must be one null-terminated UTF-8 string.
            virtual RV set_title(const c8* title) = 0;

            //! Converts one screen coordinate to one client coordinate.
            virtual Int2U screen_to_client(const Int2U& point) = 0;

            //! Converts one client coordinate to one screen coordinate.
            virtual Int2U client_to_screen(const Int2U& point) = 0;
            
            //! Gets the window events set. The user application can register callbacks to monitor 
            //! events.
            //! @return Returns one reference to the window events set.
            virtual WindowEvents& get_events() = 0;

            //! Starts receiving Unicode text input for this window.
            //! @details `input_character` window event will be triggered only after this is called and before
            //! @ref end_text_input is called.
            //! 
            //! On some platforms this call will bring up IME and on-screen virtual keyboard to let the user
            //! input texts.
            virtual RV begin_text_input() = 0;

            //! Sets the text input area, so that platform may place an IME overlay next to this area.
            //! @param[in] input_rect The input area in window coordinates.
            //! @param[in] cursor The X offset of the cursor relative to `input_rect.offset_x`.
            virtual RV set_text_input_area(const RectI& input_rect, i32 cursor) = 0;

            //! Stops receiving Unicode text input for this window.
            virtual RV end_text_input() = 0;
        };

        //! Flags that specifies the initial state and style of the window.
        enum class WindowCreationFlag : u32
        {
            none = 0x00,
            //! Window does not displayed when being created.
            hidden = 0x01,
        };

        //! Creates one new window. The new window will be displayed immediately unless @ref WindowCreationFlag::hidden is set.
        //! @param[in] title The title of the window.
        //! @param[in] x The X (left) position of the content area of window measured in screen coordinates.
        //! 
        //! If @ref DEFAULT_POS is specified, the system will choose the most suitable position for the window.
        //! @param[in] y The Y (top) position of the content area of window measured in screen coordinates.
        //! 
        //! If @ref DEFAULT_POS is specified, the system will choose the most suitable position for the window.
        //! @param[in] width The width of the window, measured in screen coordinates.
        //! 
        //! If `0` is specified, the system will choose the suitable size for the window.
        //! @param[in] height The height of the window, measured in screen coordinates.
        //! 
        //! If `0` is specified, the system will choose the suitable size for the window.
        //! @param[in] creation_flags Additional window creation flags.
        //! @return Returns the created window.
        //! @par Valid Usage
        //! * This function can only be called by the main thread.
        LUNA_WINDOW_API R<Ref<IWindow>> new_window(const c8* title, 
            i32 x = DEFAULT_POS,
            i32 y = DEFAULT_POS,
            u32 width = 0,
            u32 height = 0,
            WindowStyleFlag style_flags = WindowStyleFlag::resizable,
            WindowCreationFlag creation_flags = WindowCreationFlag::none);
        
        //! @}
    }

    LUNA_WINDOW_API Module* module_window();
}