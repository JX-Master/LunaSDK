/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file IWindowsWindow.hpp
* @author JXMaster
* @date 2022/4/5
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>

#include "../Window.hpp"

#include <objc/objc.h>

namespace Luna
{
    namespace Window
    {
        struct ICocoaWindow : virtual IWindow
        {
            luiid("{f9ac2644-eb2d-4394-b653-611bed104bb9}");

            virtual id get_nswindow() = 0;
        };
    }
}