/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file KeyCode.hpp
* @author JXMaster
* @date 2019/1/5
*/
#pragma once
#include <Luna/Runtime/KeyCode.hpp>

namespace Luna
{
    namespace HID
    {
        //! @addtogroup HID
        //! @{

        //! Compatibility alias for the core keyboard key code enum.
        //! @remark New code should use @ref Luna::KeyCode directly.
        using ::Luna::KeyCode;

        //! Specifies mouse button. These values can be bitwise-OR combined to select multiple mouse buttons if needed.
        enum class MouseButton : u8
        {
            none = 0x00,
            //! The left mouse button.
            left = 0x01,
            //! The right mouse button.
            right = 0x02,
            //! The middle mouse button.
            middle = 0x04,
            //! The function 1 mouse button.
            function1 = 0x08,
            //! The function 2 mouse button.
            function2 = 0x10,
        };

        //! @}
    }
}
