/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file EventDispatching.h
* @author JXMaster
* @date 2025/10/9
*/
#pragma once
#include "../../../Event.hpp"

namespace Luna
{
    namespace Window
    {
        struct Window;
        void post_custom_event_to_queue(Window* window, u32 event_type, u32 data1, u32 data2);
        constexpr u32 APP_DEFINED_EVENT_SHOW = 1;
        constexpr u32 APP_DEFINED_EVENT_HIDE = 2;
    }
}