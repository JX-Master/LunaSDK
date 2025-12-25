/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file UIKitWindow.hpp
* @author JXMaster
* @date 2025/11/21
*/
#pragma once
#include <Luna/Runtime/PlatformDefines.hpp>
#include "../Window.hpp"
#include <objc/objc.h>

namespace Luna
{
    namespace Window
    {
        //! @addtogroup Window
        //! @{
        
        //! @interface IUIKitWindow
        //! Implemented by window object on iOS platform.
        struct IUIKitWindow : virtual IWindow
        {
            luiid("dccb697e-4411-4797-b28d-78d1fcd0339f");

            //! Gets UIWIndow* handle of the window object.
            //! @return Returns UIWIndow* handle of the window object.
            //! The window handle is owned by the window object, the user cannot increase/decrease the reference
            //! counter of the window handle.
            virtual id get_uiwindow() = 0;

            //! Gets UIView* handle of the window object.
            //! @return Rerurns UIView* handle of the window object.
            //! The handle is owned by the window object, the user cannot increase/decrease the reference
            //! counter of the handle.
            virtual id get_uiview() = 0;
        };
    }
}