/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file IWindowsWindow.hpp
* @author JXMaster
* @date 2022/4/5
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#include "../Window.hpp"
#include "Win32Window.generated.hpp"

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{

        //! @interface IWin32Window
        //! Implemented by window object on Windows platform.
        struct [[Luna::interface("{939C8832-C687-4F8E-811B-506B62C872F0}")]] IWin32Window : virtual IWindow
        {
            //! Fetches the native handle (HWND) of this window.
            //! @return Returns the native handle of this window.
            virtual HWND get_hwnd() = 0;
        };

        //! @}
    }
}
