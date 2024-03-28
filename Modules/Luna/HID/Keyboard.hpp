/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Keyboard.hpp
* @author JXMaster
* @date 2019/7/25
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include "KeyCode.hpp"

#ifndef LUNA_HID_API
#define LUNA_HID_API
#endif

namespace Luna
{
    namespace HID
    {
        //! @addtogroup HID
        //! @{
        
        //! Checks if keyboard input is supported on the current platform.
        //! @return Returns `true` if keyboard input is supported on the current platform. Returns `false` otherwise.
        LUNA_HID_API bool supports_keyboard();

        //! Checks the if the specified key on the keyboard is pressed.
        //! @param[in] key The key to check.
        //! @return Returns `true` if the specified key is pressed, `false` otherwise.
        //! If keyboard is not supported on the current platform, returns `false` always.
        LUNA_HID_API bool get_key_state(KeyCode key);

        //! @}
    }
}