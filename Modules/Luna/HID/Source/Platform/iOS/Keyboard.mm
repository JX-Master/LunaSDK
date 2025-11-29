/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Keyboard.mm
* @author JXMaster
* @date 2025/11/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Keyboard.hpp"

namespace Luna
{
    namespace HID
    {
        // On iOS, text input is generally handled via IME and window events instead of
        // global keyboard polling, so we currently report keyboard as unsupported here.

        LUNA_HID_API bool supports_keyboard()
        {
            return false;
        }

        LUNA_HID_API bool get_key_state(KeyCode key)
        {
            // Global key state querying is not available on iOS.
            (void)key;
            return false;
        }
    }
}


