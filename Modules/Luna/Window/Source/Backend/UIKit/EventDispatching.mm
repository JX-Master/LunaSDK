/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventDispatching.mm
* @author JXMaster
* @date 2025/11/20
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT

#include "UIKitAppDelegate.h"
#include "../../Event.hpp"

namespace Luna
{
    namespace Window
    {
        // Process all pending system events.
        static void pump_events()
        {
            if(!g_pump_events) return;

            const CFTimeInterval seconds = 0.000002;

            // Pump most event types.
            SInt32 result;
            do 
            {
                result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, TRUE);
            } while (result == kCFRunLoopRunHandledSource);

            // Make sure UIScrollView objects scroll properly.
            do 
            {
                result = CFRunLoopRunInMode((CFStringRef)UITrackingRunLoopMode, seconds, TRUE);
            } while (result == kCFRunLoopRunHandledSource);
        }

        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            g_any_event_dispatched = false;
            pump_events();
            while(wait_events && !g_any_event_dispatched)
            {
                sleep(1);
                pump_events();
            }
        }
    }
}