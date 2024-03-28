/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Controller.hpp
* @author JXMaster
* @date 2019/7/25
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_HID_API
#define LUNA_HID_API
#endif

namespace Luna
{
    namespace HID
    {
        //! @addtogroup HID
        //! @{
        
        //! Specifies controller buttons. These values can be bitwise-OR combined to select multiple mouse buttons if needed.
        enum class ControllerButton : u32
        {
            none        = 0x0000,
            //! Left thumb button.
            lthumb        = 0x0001,
            //! Right thumb button.
            rthumb        = 0x0002,
            //! Up button.
            up            = 0x0004,
            //! Down button.
            down        = 0x0008,
            //! Left button.
            left        = 0x0010,
            //! Right button.
            right        = 0x0020,
            //! A button on XBOX and Nintendo controller, cross (X) button on PS controller.
            a            = 0x0040,
            //! B button on XBOX and Nintendo controller, circle (O) button on PS controller.
            b            = 0x0080,
            //! X button on XBOX and Nintendo controller, square button on PS controller.
            x            = 0x0100,
            //! Y button on XBOX and Nintendo controller, triangle button on PS controller.
            y            = 0x0200,
            //! Left shoulder button. LB for XBOX controller, L for Nintendo controller, L1 for PS controller.
            lb            = 0x0400,
            //! Right shoulder button. RB for XBOX controller, R for Nintendo controller, R1 for PS controller.
            rb            = 0x0800,
            //! Left trigger button. LT for XBOX controller, ZL for Nintendo controller, L2 for PS controller.
            lt            = 0x1000,
            //! Right trigger button. RT for XBOX controller, ZR for Nintendo controller, R2 for PS controller.
            rt            = 0x2000,
            //! Left special button.
            lspecial    = 0x4000,
            //! Right special button.
            rspecial    = 0x8000
        };

        //! The fetched input state for one generic game controller.
        struct ControllerInputState
        {
            //! Whether this device is connected and the state is valid.
            bool connected;
            //! A combination of bits to represent the pressed state of each button (1 for pressed, 0 for not pressed).
            ControllerButton buttons;
            //! The x axis for left pad, mapped to [-1, 1].
            f32 axis_lx;
            //! The y axis for left pad, mapped to [-1, 1].
            f32 axis_ly;
            //! The x axis for right pad, mapped to [-1, 1].
            f32 axis_rx;
            //! The y axis for right pad, mapped to [-1, 1].
            f32 axis_ry;
            //! The left trigger value, mapped to [0, 1].
            //! For non-linear controllers, the value is either 0 or 1.
            //! @ref ControllerButton::lt is also set if this value is greater one system-specified threshold.
            f32 axis_lt;
            //! The right trigger value, mapped to [0, 1].
            //! For non-linear controllers, the value is either 0 or 1.
            //! @ref ControllerButton::rt is also set if this value is greater one system-specified threshold.
            f32 axis_rt;
        };

        //! The state to set for a controller.
        struct ControllerOutputState
        {
            //! The vibration level for left vibration motor, mapped to [0, 1].
            f32 left_vibration;
            //! The vibration level for right vibration motor, mapped to [0, 1].
            f32 right_vibration;
        };

        //! Checks if game controller input is supported on the current platform.
        //! @return Returns `true` if game controller input is supported on the current platform, returns `false`
        //! otherwise.
        LUNA_HID_API bool supports_controller();

        //! Fetches the input state of the specified controller.
        //! @param[in] index The 0-based index of controller.
        //! @return Returns the state of the specified controller.
        //! If controller input is not supported on this platform or the controller at the specified index is not connected, 
        //! returns one structure with all values set to 0.
        LUNA_HID_API ControllerInputState get_controller_state(u32 index);

        //! Sets the output state of the specified controller.
        //! @param[in] index The 0-based index of controller.
        //! @param[in] state The state to set. This must specify one valid connected controller.
        LUNA_HID_API RV set_controller_state(u32 index, const ControllerOutputState& state);

        //! @}
    }
}