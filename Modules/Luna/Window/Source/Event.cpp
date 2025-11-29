/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.cpp
* @author JXMaster
* @date 2025/10/7
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../Event.hpp"
#include "Window.hpp"
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/TSAssert.hpp>

namespace Luna
{
    namespace Window
    {
        void(*g_event_handler)(object_t event, void* userdata) = nullptr;
        void* g_event_handler_userdata = nullptr;
        bool g_any_event_dispatched = false;

        void dispatch_event_to_handler(object_t event)
        {
            g_any_event_dispatched = true;
            if(g_event_handler)
            {
                g_event_handler(event, g_event_handler_userdata);
            }
        }

        LUNA_WINDOW_API void set_event_handler(void(*event_handler)(object_t event, void* userdata), void* userdata)
        {
            lutsassert_main_thread();
            g_event_handler = event_handler;
            g_event_handler_userdata = userdata;
        }

        LUNA_WINDOW_API void get_event_handler(void(**out_event_handler)(object_t event, void* userdata), void** out_userdata)
        {
            lutsassert_main_thread();
            if(out_event_handler) *out_event_handler = g_event_handler;
            if(out_userdata) *out_userdata = g_event_handler_userdata;
        }

        void register_events()
        {
            auto window_event_type = register_struct_type<WindowEvent>({});
            register_struct_type<WindowRequestCloseEvent>({}, window_event_type);
            register_struct_type<WindowClosedEvent>({}, window_event_type);
            register_struct_type<WindowInputFocusEvent>({}, window_event_type);
            register_struct_type<WindowLoseInputFocusEvent>({}, window_event_type);
            register_struct_type<WindowShowEvent>({}, window_event_type);
            register_struct_type<WindowHideEvent>({}, window_event_type);
            register_struct_type<WindowResizeEvent>({}, window_event_type);
            register_struct_type<WindowFramebufferResizeEvent>({}, window_event_type);
            register_struct_type<WindowMoveEvent>({}, window_event_type);
            register_struct_type<WindowDPIScaleChangedEvent>({}, window_event_type);
            register_struct_type<WindowKeyDownEvent>({}, window_event_type);
            register_struct_type<WindowKeyUpEvent>({}, window_event_type);
            register_struct_type<WindowInputTextEvent>({}, window_event_type);
            register_struct_type<WindowMouseEnterEvent>({}, window_event_type);
            register_struct_type<WindowMouseLeaveEvent>({}, window_event_type);
            register_struct_type<WindowMouseMoveEvent>({}, window_event_type);
            register_struct_type<WindowMouseDownEvent>({}, window_event_type);
            register_struct_type<WindowMouseUpEvent>({}, window_event_type);
            register_struct_type<WindowScrollEvent>({}, window_event_type);
            register_struct_type<WindowTouchDownEvent>({}, window_event_type);
            register_struct_type<WindowTouchMoveEvent>({}, window_event_type);
            register_struct_type<WindowTouchUpEvent>({}, window_event_type);
            register_struct_type<WindowDropFilesEvent>({}, window_event_type);
            auto app_event_type = register_struct_type<ApplicationEvent>({});
            register_struct_type<ApplicationDidEnterForegroundEvent>({}, app_event_type);
            register_struct_type<ApplicationWillEnterForegroundEvent>({}, app_event_type);
            register_struct_type<ApplicationDidEnterBackgroundEvent>({}, app_event_type);
            register_struct_type<ApplicationWillEnterBackgroundEvent>({}, app_event_type);
            register_struct_type<ApplicationWillTerminateEvent>({}, app_event_type);
            register_struct_type<ApplicationDidReceiveMemoryWarningEvent>({}, app_event_type);
            register_struct_type<ScreenKeyboardShownEvent>({});
            register_struct_type<ScreenKeyboardHiddenEvent>({});
        }
    }
}