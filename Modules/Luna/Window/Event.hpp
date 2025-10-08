/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.hpp
* @author JXMaster
* @date 2025/10/6
*/
#pragma once
#include "Window.hpp"

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{

        //! Sets the function for handling events fetched by @ref poll_events.
        //! @param[in] event_handler The function that will be called for handling each fetched event.
        //! @param[in] userdata The user-defined data that will be passed to `event_handler`.
        LUNA_WINDOW_API void set_event_handler(void(*event_handler)(object_t event, void* userdata), void* userdata);

        //! Gets the function for handling events fetched by @ref poll_events.
        //! @param[out] out_event_handler Returns the function that is set for handling each fetched event.
        //! @param[out] out_userdata Returns the user-defined data that will be passed to `
        LUNA_WINDOW_API void get_event_handler(void(**out_event_handler)(object_t event, void* userdata), void** out_userdata);

        //! Processes all application events in the event queue.
        //! @param[in] wait_events Whether to suspend the current thread until one event is 
        //! fetched if the event queue is empty.
        LUNA_WINDOW_API void poll_events(bool wait_events = false);

        //! The base class for all events dispatched by to a specific window.
        struct WindowEvent
        {
            lustruct("Window::WindowEvent", "749dcf28-511b-430f-810e-e09bcd98652f");

            //! The window that this event is dispatched to.
            Ref<IWindow> window;
        };

        //! Dispatched when one window is requested to be closed, usually
        //! because the user clicks the close button of the window.
        //! 
        //! The application can call @ref IWindow::close to close this window.
        //! @par Default Behavior
        //! Call @ref IWindow::close.
        struct WindowRequestCloseEvent : WindowEvent
        {
            lustruct("Window::WindowRequestCloseEvent", "dbae6f99-e921-4df2-97b7-2876644dedee");

            //! Set this to `true` if the window should be closed by this close request.
            //! Set this to `false` otherwise.
            //! The default value is `true`.
            bool do_close;
        };

        //! Dispatched when one window is closed. The handler should clean up any resource 
        //! attached to this window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowClosedEvent : WindowEvent
        {
            lustruct("Window::WindowClosedEvent", "46c9952b-7bdd-4aad-b740-529ab35847dc");

            // No additional members.
        };

        //! Dispatched when one window gains input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowInputFocusEvent : WindowEvent
        {
            lustruct("Window::WindowInputFocusEvent", "e1c9b77f-edb2-4b6e-84d1-d75141dee2ab");

            // No additional members.
        };

        //! Dispatched when one window loses input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowLoseInputFocusEvent : WindowEvent
        {
            lustruct("Window::WindowLoseInputFocusEvent", "6c67f7c0-70b7-46b8-98d6-4ebd954d17f0");

            // No additional members.
        };

        //! Dispatched when the window's visibility is changed from hidden to show.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowShowEvent : WindowEvent
        {
            lustruct("Window::WindowShowEvent", "b5c31f80-d3a0-4c8b-b1f1-6de8f5ac7e45");

            // No additional members.
        };

        //! Dispatched when the window's visibility is changed from show to hidden.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowHideEvent : WindowEvent
        {
            lustruct("Window::WindowHideEvent", "c2d7be3b-173a-4f3a-b920-faa8a4d36e20");

            // No additional members.
        };

        //! Dispatched when the window's size is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowResizeEvent : WindowEvent
        {
            lustruct("Window::WindowResizeEvent", "23e8b2b2-ad63-4030-95fc-dbd6d8e766db");

            //! The new width of the window in screen coordinates.
            u32 width;
            //! The new height of the window in screen coordinates.
            u32 height;
        };

        //! Dispatched when the window's framebuffer size is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowFramebufferResizeEvent : WindowEvent
        {
            lustruct("Window::WindowFramebufferResizeEvent", "fb7c9bbd-9805-46c6-abe8-4800d842b2c9");

            //! The new width of the window's framebuffer size in pixels.
            u32 width;
            //! The new height of the window's framebuffer size in pixels.
            u32 height;
        };

        //! Dispatched when the window's position is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMoveEvent : WindowEvent
        {
            lustruct("Window::WindowMoveEvent", "68b51b18-0f9d-4a0c-99b8-3b899f1cac9c");

            //! The X position of the window in screen coordinates after move.
            i32 x;
            //! The Y position of the window in screen coordinates after move.
            i32 y;
        };

        //! Dispatched when the window's DPI (dots per inch) is changed. This 
        //! may happen if the user changes the DPI of the display, or moves the window to another display
        //! with different DPI.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowDPIScaleChangedEvent : WindowEvent
        {
            lustruct("Window::WindowDPIScaleChangedEvent", "561beeb5-3f0a-4bf6-a398-84eddcf2a5e1");

            // No additional members.
        };

        //! Dispatched when the user presses one key with one window being focused.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowKeyDownEvent : WindowEvent
        {
            lustruct("Window::WindowKeyDownEvent", "4ebe2c21-79e9-4ec8-addb-c1fe6e5a9236");

            //! The key that is pressed.
            HID::KeyCode key;
        };

        //! Dispatched when the user releases one key with one window being focused.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowKeyUpEvent : WindowEvent
        {
            lustruct("Window::WindowKeyUpEvent", "9deb9e17-86aa-408c-bb1a-71793646dce5");

            //! The key that is released.
            HID::KeyCode key;
        };

        //! Dispatched when the window receives input text.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowInputTextEvent : WindowEvent
        {
            lustruct("Window::WindowInputTextEvent", "b817bef2-44ed-480c-9f38-56a86d7d9bc0");

            //! The input text in UTF-8 encoding. The text is null-terminated.
            String text;
            //! The length of the input text.
            usize length;
        };

        //! Dispatched when the mouse cursor enters the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseEnterEvent : WindowEvent
        {
            lustruct("Window::WindowMouseEnterEvent", "48f45132-8a92-4392-bf03-e55d3544a814");

            // No additional members.
        };

        //! Dispatched when the mouse cursor leaves the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseLeaveEvent : WindowEvent
        {
            lustruct("Window::WindowMouseLeaveEvent", "d25080bb-4b4c-409e-b856-40282d00c795");

            // No additional members.
        };

        //! Dispatched when the mouse cursor is moved in the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseMoveEvent : WindowEvent
        {
            lustruct("Window::WindowMouseMoveEvent", "051246ab-55dc-4e05-89a0-44a1d95e5319");

            //! The new X position of the mouse cursor relative to the window client area.
            i32 x;
            //! The new Y position of the mouse cursor relative to the window client area.
            i32 y;
        };

        //! Dispatched when the mouse button is pressed and the target window has mouse input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseDownEvent : WindowEvent
        {
            lustruct("Window::WindowMouseDownEvent", "c66b5922-e5f4-46fe-9b3e-4299b0a35241");

            //! The mouse button that is pressed.
            HID::MouseButton button;
        };

        //! Dispatched when the mouse button is released and the target window has mouse input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseUpEvent : WindowEvent
        {
            lustruct("Window::WindowMouseUpEvent", "0d307596-4b17-4e6e-8091-b7a53d9a813c");

            //! The mouse button that is released.
            HID::MouseButton button;
        };

        //! Dispatched when the window is scrolled by mouse wheel or trackpad and the window gains input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowScrollEvent : WindowEvent
        {
            lustruct("Window::WindowScrollEvent", "4aed1809-d4ec-4406-a90f-b07d05d3bab3");

            //! The scroll delta in X dimension.
            f32 scroll_x;
            //! The scroll delta in Y dimension.
            f32 scroll_y;
        };

        //! Dispatched when a new touch point is detected.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowTouchDownEvent : WindowEvent
        {
            lustruct("Window::WindowTouchDownEvent", "e96554e5-d807-4c41-a1bb-62b18c1f015b");

            //! The identifier of the touch point. 
            //! This id remains unchanged for the same touch point during different touch events.
            u64 id;
            //! The x position of the touch point relative to the window position.
            f32 x;
            //! The y position of the touch point relative to the window position.
            f32 y;
        };

        //! Dispatched when the position of one existing touch point is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowTouchMoveEvent : WindowEvent
        {
            lustruct("Window::WindowTouchMoveEvent", "a77f1f7f-26e9-4ce2-8d9b-9677fa65f50f");

            //! The identifier of the touch point. 
            //! This id remains unchanged for the same touch point during different touch events.
            u64 id;
            //! The x position of the touch point relative to the window position.
            f32 x;
            //! The y position of the touch point relative to the window position.
            f32 y;
        };

        //! Dispatched when an existing touch point is released.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowTouchUpEvent : WindowEvent
        {
            lustruct("Window::WindowTouchUpEvent", "38c25e24-6c7f-4f90-91f1-133baa030b38");

            //! The identifier of the touch point. 
            //! This id remains unchanged for the same touch point during different touch events.
            u64 id;
            //! The x position of the touch point relative to the window position.
            f32 x;
            //! The y position of the touch point relative to the window position.
            f32 y;
        };

        //! Dispatched when the user drags and drops files into the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowDropFilesEvent : WindowEvent
        {
            lustruct("Window::WindowDropFilesEvent", "0127a403-7809-4ada-90d8-14a062052a67");

            //! The array of paths of files being dropped.
            Array<String> files;
            //! The x position of the drop point relative to the window position.
            f32 x;
            //! The y position of the drop point relative to the window position.
            f32 y;
        };

        //! @}
    }
}

