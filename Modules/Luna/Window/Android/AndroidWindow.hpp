/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AndroidWindow.hpp
* @author JXMaster
* @date 2025/11/25
*/
#pragma once
#include "../Window.hpp"

namespace Luna
{
    namespace Window
    {
        struct IAndroidWindow : IWindow
        {
            luiid("4e182d26-3583-4908-b745-8adb7c0bb51a");

            //! Gets ANativeWindow* pointer.
            //! @return Returns one pointer that can be casted to ANativeWindow*.
            virtual opaque_t get_native_window() = 0;
        };
    }
}