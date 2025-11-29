/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainAndroid.hpp
* @author JXMaster
* @date 2025/11/25
*/
#pragma once
#include <stdint.h>

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

extern "C"
{
    typedef struct android_app android_app;
}

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API void prepare_app(android_app *pApp);
    }
}