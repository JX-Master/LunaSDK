/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventHandling.hpp
* @author JXMaster
* @date 2025/10/5
*/
#pragma once

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {   
        //! Polls and dispatches all pending window events.
        //! This should be called from the main thread.
        LUNA_WINDOW_API void poll_cocoa_events();

        //! Handles one Cocoa NSEvent and dispatches it to the appropriate window.
        //! @param[in] nsevent_ptr Pointer to NSEvent object.
        LUNA_WINDOW_API void handle_cocoa_event(void* nsevent_ptr);
    }
}

