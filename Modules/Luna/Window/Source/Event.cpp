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
#include "Event.hpp"
#include "Window.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/TSAssert.hpp>

namespace Luna
{
    namespace Window
    {
        void(*g_event_handler)(object_t event, void* userdata) = nullptr;
        void* g_event_handler_userdata = nullptr;
        bool g_any_event_dispatched = false;
#if defined(LUNA_PLATFORM_MACOS)
        bool g_application_quit_requested = false;
#endif

        void dispatch_event_to_handler(object_t event)
        {
            g_any_event_dispatched = true;
            if(g_event_handler)
            {
                g_event_handler(event, g_event_handler_userdata);
            }
        }

#if defined(LUNA_PLATFORM_MACOS)
        void dispatch_application_menu_item_invoked(application_menu_item_id_t item_id)
        {
            auto event = new_object<ApplicationMenuItemInvokedEvent>();
            event->item_id = item_id;
            dispatch_event_to_handler(event.object());
        }

        bool dispatch_application_quit_request()
        {
            auto event = new_object<ApplicationRequestQuitEvent>();
            dispatch_event_to_handler(event.object());
            if(event->do_quit)
            {
                g_application_quit_requested = true;
            }
            return event->do_quit;
        }

        void reset_application_quit_request()
        {
            g_application_quit_requested = false;
        }
#endif

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

#if defined(LUNA_PLATFORM_MACOS)
        LUNA_WINDOW_API bool is_application_quit_requested()
        {
            lutsassert_main_thread();
            return g_application_quit_requested;
        }
#endif

    }
}
