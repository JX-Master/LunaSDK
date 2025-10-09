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
    }
}