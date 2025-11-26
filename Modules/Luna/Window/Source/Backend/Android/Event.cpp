/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.cpp
* @author JXMaster
* @date 2025/11/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../../Event.hpp"
#include <Luna/Runtime/TSAssert.hpp>

extern "C"
{
#include "../../../Android/native_app_glue/android_native_app_glue.h"
}

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API void poll_events(bool wait_events)
        {
            lutsassert_main_thread();
            int ident;
            int events;
            android_poll_source* source = nullptr;

            // If waiting, block for at least one event first.
            if (wait_events)
            {
                int res = ALooper_pollOnce(-1, &ident, &events, (void**)&source);
                if (res >= 0 && source)
                {
                    // android_poll_source carries its app pointer.
                    source->process(source->app, source);
                }
            }

            // Drain any remaining events without blocking.
            for (;;)
            {
                int res = ALooper_pollOnce(0, &ident, &events, (void**)&source);
                if (res == ALOOPER_POLL_TIMEOUT || res == ALOOPER_POLL_ERROR)
                {
                    break;
                }
                if (source)
                {
                    source->process(source->app, source);
                }
            }
        }
    }
}