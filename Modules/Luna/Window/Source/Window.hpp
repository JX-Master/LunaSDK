/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Window.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#include "../Application.hpp"
namespace Luna
{
    namespace Window
    {
        extern StartupParams g_startup_params;
        void init_events();
        void close_events();
        RV platform_init();
        void platform_close();
        void platform_poll_events(bool wait_event);
    }
}