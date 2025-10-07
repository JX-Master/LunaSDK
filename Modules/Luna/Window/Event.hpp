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

        //! Pops one event from the application's event queue and fetches the event.
        //! @param[in] wait_event Whether to suspend the current thread until one event is 
        //! fetched if the event queue is empty.
        //! @return Returns the popped event object, or null if the event queue is empty 
        //! and no event is fetched.
        LUNA_WINDOW_API ObjRef pop_event(bool wait_event = false);

        //! Pushes one event to the application's event queue.
        //! @param[in] event The event to be pushed.
        LUNA_WINDOW_API void push_event(object_t event);

        //! Handles the event using the system's default behavior.
        //! The application should call this to handle event if the application does not handle the event.
        //! @param[in] event The event to be handled.
        LUNA_WINDOW_API void default_event_handler(object_t event);

        //! Dispatched when one window is requested to be closed, usually
        //! because the user clicks the close button of the window.
        //! 
        //! The application can call @ref IWindow::close to close this window.
        //! @par Default Behavior
        //! Call @ref IWindow::close.
        struct WindowRequestCloseEvent
        {
            lustruct("Window::WindowRequestCloseEvent", "dbae6f99-e921-4df2-97b7-2876644dedee");

            //! The window that is requested to be closed.
            Ref<IWindow> window;
        };

        //! Dispatched when one window is closed. The handler should clean up any resource 
        //! attached to this window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowClosedEvent
        {
            lustruct("Window::WindowClosedEvent", "46c9952b-7bdd-4aad-b740-529ab35847dc");

            //! The window that is closed.
            Ref<IWindow> window;
        };

        //! Dispatched when one window gains input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowInputFocusEvent
        {
            lustruct("Window::WindowInputFocusEvent", "e1c9b77f-edb2-4b6e-84d1-d75141dee2ab");

            //! The window that gains the input focus.
            Ref<IWindow> window;
        };

        //! Dispatched when one window loses input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowLoseInputFocusEvent
        {
            lustruct("Window::WindowLoseInputFocusEvent", "6c67f7c0-70b7-46b8-98d6-4ebd954d17f0");

            //! The window that loses the input focus.
            Ref<IWindow> window;
        };

        //! Dispatched when the window's visibility is changed from hidden to show.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowShowEvent
        {
            lustruct("Window::WindowShowEvent", "b5c31f80-d3a0-4c8b-b1f1-6de8f5ac7e45");

            //! The window whose visibility is changed.
            Ref<IWindow> window;
        };

        //! Dispatched when the window's visibility is changed from show to hidden.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowHideEvent
        {
            lustruct("Window::WindowHideEvent", "c2d7be3b-173a-4f3a-b920-faa8a4d36e20");

            //! The window whose visibility is changed.
            Ref<IWindow> window;
        };

        //! Dispatched when the window's size is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowResizeEvent
        {
            lustruct("Window::WindowResizeEvent", "23e8b2b2-ad63-4030-95fc-dbd6d8e766db");

            //! The window whose size is changed.
            Ref<IWindow> window;
            //! The new width of the window in screen coordinates.
            u32 width;
            //! The new height of the window in screen coordinates.
            u32 height;
        };

        //! Dispatched when the window's framebuffer size is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowFramebufferResizeEvent
        {
            lustruct("Window::WindowFramebufferResizeEvent", "fb7c9bbd-9805-46c6-abe8-4800d842b2c9");

            //! The window whose framebuffer's size is changed.
            Ref<IWindow> window;
            //! The new width of the window's framebuffer size in pixels.
            u32 width;
            //! The new height of the window's framebuffer size in pixels.
            u32 height;
        };

        //! Dispatched when the window's position is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMoveEvent
        {
            lustruct("Window::WindowMoveEvent", "68b51b18-0f9d-4a0c-99b8-3b899f1cac9c");

            //! The window that is moved.
            Ref<IWindow> window;
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
        struct WindowDPIScaleChangedEvent
        {
            lustruct("Window::WindowDPIScaleChangedEvent", "561beeb5-3f0a-4bf6-a398-84eddcf2a5e1");

            //! The window whose DPI is changed.
            Ref<IWindow> window;
        };

        //! Dispatched when the user presses one key with one window being focused.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowKeyDownEvent
        {
            lustruct("Window::WindowKeyDownEvent", "4ebe2c21-79e9-4ec8-addb-c1fe6e5a9236");

            //! The window that gains keyboard input focus.
            Ref<IWindow> window;
            //! The key that is pressed.
            HID::KeyCode key;
        };

        //! Dispatched when the user releases one key with one window being focused.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowKeyUpEvent
        {
            lustruct("Window::WindowKeyUpEvent", "9deb9e17-86aa-408c-bb1a-71793646dce5");

            //! The window that gains keyboard input focus.
            Ref<IWindow> window;
            //! The key that is released.
            HID::KeyCode key;
        };

        //! Dispatched when the window receives input text.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowInputTextEvent
        {
            lustruct("Window::WindowInputTextEvent", "b817bef2-44ed-480c-9f38-56a86d7d9bc0");

            //! The window that receives the input character.
            Ref<IWindow> window;
            //! The input text in UTF-8 encoding. The text is null-terminated.
            String text;
            //! The length of the input text.
            usize length;
        };

        //! Dispatched when the mouse cursor enters the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseEnterEvent
        {
            lustruct("Window::WindowMouseEnterEvent", "48f45132-8a92-4392-bf03-e55d3544a814");

            //! The window that the mouse cursor is entered.
            Ref<IWindow> window;
        };

        //! Dispatched when the mouse cursor leaves the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseLeaveEvent
        {
            lustruct("Window::WindowMouseLeaveEvent", "d25080bb-4b4c-409e-b856-40282d00c795");

            //! The window that the mouse cursor is leaved.
            Ref<IWindow> window;
        };

        //! Dispatched when the mouse cursor is moved in the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseMoveEvent
        {
            lustruct("Window::WindowMouseMoveEvent", "051246ab-55dc-4e05-89a0-44a1d95e5319");

            //! The window that the mouse cursor is moved within.
            Ref<IWindow> window;
            //! The new X position of the mouse cursor relative to the window client area.
            i32 x;
            //! The new Y position of the mouse cursor relative to the window client area.
            i32 y;
        };

        //! Dispatched when the mouse button is pressed and the target window has mouse input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseDownEvent
        {
            lustruct("Window::WindowMouseDownEvent", "c66b5922-e5f4-46fe-9b3e-4299b0a35241");

            //! The window that gains input focus.
            Ref<IWindow> window;
            //! The mouse button that is pressed.
            HID::MouseButton button;
        };

        //! Dispatched when the mouse button is released and the target window has mouse input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowMouseUpEvent
        {
            lustruct("Window::WindowMouseUpEvent", "0d307596-4b17-4e6e-8091-b7a53d9a813c");

            //! The window that gains input focus.
            Ref<IWindow> window;
            //! The mouse button that is released.
            HID::MouseButton button;
        };

        //! Dispatched when the window is scrolled by mouse wheel or trackpad and the window gains input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowScrollEvent
        {
            lustruct("Window::WindowScrollEvent", "4aed1809-d4ec-4406-a90f-b07d05d3bab3");

            //! The window that gains input focus.
            Ref<IWindow> window;
            //! The scroll delta in X dimension.
            f32 scroll_x;
            //! The scroll delta in Y dimension.
            f32 scroll_y;
        };

        //! Dispatched when a new touch point is detected.
        //! @par Default Behavior
        //! Do nothing.
        struct WindowTouchDownEvent
        {
            lustruct("Window::WindowTouchDownEvent", "e96554e5-d807-4c41-a1bb-62b18c1f015b");

            //! The window this event is dispatched to.
            Ref<IWindow> window;
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
        struct WindowTouchMoveEvent
        {
            lustruct("Window::WindowTouchMoveEvent", "a77f1f7f-26e9-4ce2-8d9b-9677fa65f50f");

            //! The window this event is dispatched to.
            Ref<IWindow> window;
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
        struct WindowTouchUpEvent
        {
            lustruct("Window::WindowTouchUpEvent", "38c25e24-6c7f-4f90-91f1-133baa030b38");

            //! The window this event is dispatched to.
            Ref<IWindow> window;
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
        struct WindowDropFilesEvent
        {
            lustruct("Window::WindowDropFilesEvent", "0127a403-7809-4ada-90d8-14a062052a67");

            //! The window this event is dispatched to.
            Ref<IWindow> window;
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

