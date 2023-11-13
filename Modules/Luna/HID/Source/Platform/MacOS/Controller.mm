/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Controller.mm
* @author JXMaster
* @date 2023/11/13
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Controller.hpp"

namespace Luna
{
    namespace HID
    {
        LUNA_HID_API bool supports_controller()
        {
            return false;
        }
        LUNA_HID_API ControllerInputState get_controller_state(u32 index)
        {
            ControllerInputState r;
            memzero(&r);
            return r;
        }
        LUNA_HID_API RV set_controller_state(u32 index, const ControllerOutputState& state)
        {
            return BasicError::not_supported();
        }
    }
}