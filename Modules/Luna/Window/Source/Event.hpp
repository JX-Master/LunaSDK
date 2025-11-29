/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.hpp
* @author JXMaster
* @date 2025/10/9
*/
#pragma once
#include "../Event.hpp"

namespace Luna
{
    namespace Window
    {
        void dispatch_event_to_handler(object_t event);
        // Used to check whether any event is dispatched in the current event iteration.
        // If this is false and wait_events is true when calling poll_events, we should wait for the event.
        // This is used only for iOS and Android, since Windows/macOS has native API for waiting events.
        extern bool g_any_event_dispatched;
    }
}