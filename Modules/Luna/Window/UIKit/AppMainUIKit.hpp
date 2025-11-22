/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AppMainUIKit.hpp
* @author JXMaster
* @date 2025/11/19
*/
#pragma once

#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif

namespace Luna
{
    namespace Window
    {
        //! Runs UIKit application.
        LUNA_WINDOW_API int uikit_app_run(int argc, char *argv[], int (*luna_main_func)(int argc, const char* argv[]));
    }
}