/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file GLFWWindow.hpp
* @author JXMaster
* @date 2022/4/5
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>
#include "../Window.hpp"

struct GLFWwindow;

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{

        //! @interface IGLFWWindow
        //! Implemented by the window object if the underlying window is a GLFW window.
        //! This interface is enabled on Windows, Linux and macOS platform.
        struct IGLFWWindow : virtual IWindow
        {
            luiid("{31E69084-EE4C-40B1-A4B7-37F4B7C04472}");

            //! Gets GLFWwindow* handle of this window.
            //! @return Returns the GLFW handle of this window.
            virtual GLFWwindow* get_glfw_window_handle() = 0;
        };

        //! @}
    }
}