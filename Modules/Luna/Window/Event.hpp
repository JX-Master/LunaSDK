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
#include "Application.hpp"
#include "ApplicationMenu.hpp"
#include <Luna/HID/KeyCode.hpp>
#include "Event.generated.hpp"
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

        //! Checks whether an application quit request was accepted by the event handler.
        //! @return Returns `true` if a quit request was accepted since the Window module was initialized.
        //! @remark An accepted quit request remains set until the Window module is reinitialized. The application
        //! should observe this state, release its resources and return normally through `luna_main`.
        //! @par Valid Usage
        //! * This function must be called from the main thread after the Window module is initialized.
        LUNA_WINDOW_API bool is_application_quit_requested();

        //! The base class for all events dispatched by to a specific window.
        struct [[luna::struct("749dcf28-511b-430f-810e-e09bcd98652f")]] WindowEvent
        {
            //! The window that this event is dispatched to.
            Ref<IWindow> window;
        };

        //! Dispatched when one window is requested to be closed, usually
        //! because the user clicks the close button of the window.
        //! 
        //! The application can call @ref IWindow::close to close this window.
        //! @par Default Behavior
        //! Call @ref IWindow::close.
        struct [[luna::struct("dbae6f99-e921-4df2-97b7-2876644dedee")]] WindowRequestCloseEvent : WindowEvent
        {
            //! Set this to `true` if the window should be closed by this close request.
            //! Set this to `false` otherwise.
            //! The default value is `true`.
            bool do_close;
        };

        //! Dispatched when one window is closed. The handler should clean up any resource 
        //! attached to this window.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("46c9952b-7bdd-4aad-b740-529ab35847dc")]] WindowClosedEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when one window gains input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("e1c9b77f-edb2-4b6e-84d1-d75141dee2ab")]] WindowInputFocusEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when one window loses input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("6c67f7c0-70b7-46b8-98d6-4ebd954d17f0")]] WindowLoseInputFocusEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when the window's visibility is changed from hidden to show.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("b5c31f80-d3a0-4c8b-b1f1-6de8f5ac7e45")]] WindowShowEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when the window's visibility is changed from show to hidden.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("c2d7be3b-173a-4f3a-b920-faa8a4d36e20")]] WindowHideEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when the window's size is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("23e8b2b2-ad63-4030-95fc-dbd6d8e766db")]] WindowResizeEvent : WindowEvent
        {
            //! The new width of the window in screen coordinates.
            u32 width;
            //! The new height of the window in screen coordinates.
            u32 height;
        };

        //! Dispatched when the window's framebuffer size is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("fb7c9bbd-9805-46c6-abe8-4800d842b2c9")]] WindowFramebufferResizeEvent : WindowEvent
        {
            //! The new width of the window's framebuffer size in pixels.
            u32 width;
            //! The new height of the window's framebuffer size in pixels.
            u32 height;
        };

        //! Dispatched when the window's position is changed.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("68b51b18-0f9d-4a0c-99b8-3b899f1cac9c")]] WindowMoveEvent : WindowEvent
        {
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
        struct [[luna::struct("561beeb5-3f0a-4bf6-a398-84eddcf2a5e1")]] WindowDPIScaleChangedEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when the user presses one key with one window being focused.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("4ebe2c21-79e9-4ec8-addb-c1fe6e5a9236")]] WindowKeyDownEvent : WindowEvent
        {
            //! The key that is pressed.
            KeyCode key;
        };

        //! Dispatched when the user releases one key with one window being focused.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("9deb9e17-86aa-408c-bb1a-71793646dce5")]] WindowKeyUpEvent : WindowEvent
        {
            //! The key that is released.
            KeyCode key;
        };

        //! Dispatched when the window receives input text.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("b817bef2-44ed-480c-9f38-56a86d7d9bc0")]] WindowInputTextEvent : WindowEvent
        {
            //! The input text in UTF-8 encoding. The text is null-terminated.
            String text;
        };

        //! Dispatched when the mouse cursor enters the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("48f45132-8a92-4392-bf03-e55d3544a814")]] WindowMouseEnterEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when the mouse cursor leaves the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("d25080bb-4b4c-409e-b856-40282d00c795")]] WindowMouseLeaveEvent : WindowEvent
        {
            // No additional members.
        };

        //! Dispatched when the mouse cursor is moved in the non-covered region of the window.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("051246ab-55dc-4e05-89a0-44a1d95e5319")]] WindowMouseMoveEvent : WindowEvent
        {
            //! The new X position of the mouse cursor relative to the window client area.
            i32 x;
            //! The new Y position of the mouse cursor relative to the window client area.
            i32 y;
        };

        //! Dispatched when the mouse button is pressed and the target window has mouse input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("c66b5922-e5f4-46fe-9b3e-4299b0a35241")]] WindowMouseDownEvent : WindowEvent
        {
            //! The mouse button that is pressed.
            HID::MouseButton button;
        };

        //! Dispatched when the mouse button is released and the target window has mouse input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("0d307596-4b17-4e6e-8091-b7a53d9a813c")]] WindowMouseUpEvent : WindowEvent
        {
            //! The mouse button that is released.
            HID::MouseButton button;
        };

        //! Dispatched when the window is scrolled by mouse wheel or trackpad and the window gains input focus.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("4aed1809-d4ec-4406-a90f-b07d05d3bab3")]] WindowScrollEvent : WindowEvent
        {
            //! The scroll delta in X dimension.
            f32 scroll_x;
            //! The scroll delta in Y dimension.
            f32 scroll_y;
        };

        //! Dispatched when a new touch point is detected.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("e96554e5-d807-4c41-a1bb-62b18c1f015b")]] WindowTouchDownEvent : WindowEvent
        {
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
        struct [[luna::struct("a77f1f7f-26e9-4ce2-8d9b-9677fa65f50f")]] WindowTouchMoveEvent : WindowEvent
        {
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
        struct [[luna::struct("38c25e24-6c7f-4f90-91f1-133baa030b38")]] WindowTouchUpEvent : WindowEvent
        {
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
        struct [[luna::struct("0127a403-7809-4ada-90d8-14a062052a67")]] WindowDropFilesEvent : WindowEvent
        {
            //! The array of paths of files being dropped.
            Array<String> files;
            //! The x position of the drop point relative to the window position.
            f32 x;
            //! The y position of the drop point relative to the window position.
            f32 y;
        };

        //! The base class for all application level events.
        struct [[luna::struct("ce988e37-c317-4fac-8538-726df3a6a62b")]] ApplicationEvent
        {
        };

        //! Dispatched when the user invokes one application-defined menu item.
        //! @par Default Behavior
        //! Do nothing.
        struct [[luna::struct("5a2acc84-ecde-443c-b75d-63de9c5b8150")]] ApplicationMenuItemInvokedEvent : ApplicationEvent
        {
            //! The identifier of the invoked menu item.
            application_menu_item_id_t item_id = 0;
        };

        //! Dispatched when the user or platform requests the application to quit.
        //! @details The application can reject the request, for example while prompting the user to save unsaved
        //! work. An accepted request is recorded and can be queried by @ref is_application_quit_requested.
        //! @par Default Behavior
        //! Accept the request.
        struct [[luna::struct("7beab768-33a0-46f2-be2b-daea476a157f")]] ApplicationRequestQuitEvent : ApplicationEvent
        {
            //! Set this to `true` to accept the quit request, or `false` to reject it.
            //! The default value is `true`.
            bool do_quit = true;
        };

        //! Dispatched when the application has entered foreground.
        struct [[luna::struct("5bb08e54-ac48-47be-9487-4221dcb26d6d")]] ApplicationDidEnterForegroundEvent : ApplicationEvent
        {
        };

        //! Dispatched when the application will enter foreground.
        struct [[luna::struct("43bee904-7ca8-441c-9c4c-fff2184170db")]] ApplicationWillEnterForegroundEvent : ApplicationEvent
        {
        };

        //! Dispatched when the applicaiton has entered background.
        struct [[luna::struct("3bc3cf7b-99c9-4e0e-b612-903758eb292e")]] ApplicationDidEnterBackgroundEvent : ApplicationEvent
        {
        };

        //! Dispatched when the application will enter background.
        struct [[luna::struct("08a8d9e7-4714-495d-9311-e89107185817")]] ApplicationWillEnterBackgroundEvent : ApplicationEvent
        {
        };

        //! Dispatched when the application is about to be terminated.
        struct [[luna::struct("be1ebcea-348b-4e16-8849-a30b1d395a52")]] ApplicationWillTerminateEvent : ApplicationEvent
        {
        };

        //! Dispatched when the system memory is low and the application should
        //! free some memory to prevent being killed.
        struct [[luna::struct("ae40df0e-b372-46f5-90ad-d363ce3ed89a")]] ApplicationDidReceiveMemoryWarningEvent : ApplicationEvent
        {
        };

        //! Dispatched when the screen keyboard is shown.
        struct [[luna::struct("07d9ab5b-27bf-4227-b052-88d7141c72ea")]] ScreenKeyboardShownEvent
        {
        };

        //! Dispatched when the screen keyboard is hidden.
        struct [[luna::struct("4c1aa6c9-01ea-4603-89f3-63aef717b23a")]] ScreenKeyboardHiddenEvent
        {
        };

        //! @}
    }
}
