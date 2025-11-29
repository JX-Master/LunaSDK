/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Controller.mm
* @author JXMaster
* @date 2025/11/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_HID_API LUNA_EXPORT
#include "../../../Controller.hpp"
#import <GameController/GameController.h>

namespace Luna
{
    namespace HID
    {
        static void fill_from_extended(GCController* controller, GCExtendedGamepad* gp, ControllerInputState& state)
        {
            // DPad.
            if(gp.dpad.up.pressed) set_flags(state.buttons, ControllerButton::up);
            if(gp.dpad.down.pressed) set_flags(state.buttons, ControllerButton::down);
            if(gp.dpad.left.pressed) set_flags(state.buttons, ControllerButton::left);
            if(gp.dpad.right.pressed) set_flags(state.buttons, ControllerButton::right);

            // Face buttons.
            if(gp.buttonA.pressed) set_flags(state.buttons, ControllerButton::a);
            if(gp.buttonB.pressed) set_flags(state.buttons, ControllerButton::b);
            if(gp.buttonX.pressed) set_flags(state.buttons, ControllerButton::x);
            if(gp.buttonY.pressed) set_flags(state.buttons, ControllerButton::y);

            // Shoulders.
            if(gp.leftShoulder.pressed) set_flags(state.buttons, ControllerButton::lb);
            if(gp.rightShoulder.pressed) set_flags(state.buttons, ControllerButton::rb);

            // Triggers.
            state.axis_lt = gp.leftTrigger.value;
            state.axis_rt = gp.rightTrigger.value;
            constexpr f32 TRIGGER_THRESHOLD = 0.1f;
            if(state.axis_lt > TRIGGER_THRESHOLD) set_flags(state.buttons, ControllerButton::lt);
            if(state.axis_rt > TRIGGER_THRESHOLD) set_flags(state.buttons, ControllerButton::rt);

            // Sticks.
            state.axis_lx = gp.leftThumbstick.xAxis.value;
            state.axis_ly = gp.leftThumbstick.yAxis.value;
            state.axis_rx = gp.rightThumbstick.xAxis.value;
            state.axis_ry = gp.rightThumbstick.yAxis.value;

            if(@available(iOS 12.1, *))
            {
                if(gp.leftThumbstickButton && gp.leftThumbstickButton.pressed)
                {
                    set_flags(state.buttons, ControllerButton::lthumb);
                }
                if(gp.rightThumbstickButton && gp.rightThumbstickButton.pressed)
                {
                    set_flags(state.buttons, ControllerButton::rthumb);
                }
            }

            if(@available(iOS 13.0, *))
            {
                if(gp.buttonMenu.pressed)
                {
                    set_flags(state.buttons, ControllerButton::rspecial);
                }
                if(gp.buttonOptions && gp.buttonOptions.pressed)
                {
                    set_flags(state.buttons, ControllerButton::lspecial);
                }
            }
        }

        static void fill_from_micro_gamepad(GCController* controller, GCMicroGamepad* gp, ControllerInputState& state)
        {
            // Map micro dpad to left stick axes.
            state.axis_lx = gp.dpad.xAxis.value;
            state.axis_ly = gp.dpad.yAxis.value;

            if(gp.buttonA.pressed) set_flags(state.buttons, ControllerButton::a);
            if(gp.buttonX.pressed) set_flags(state.buttons, ControllerButton::x);
        }

        LUNA_HID_API bool supports_controller()
        {
            // iOS with GameController framework supports game controllers.
            return true;
        }

        LUNA_HID_API ControllerInputState get_controller_state(u32 index)
        {
            ControllerInputState ret;
            memzero(&ret, sizeof(ControllerInputState));
            @autoreleasepool
            {
                NSArray<GCController*>* controllers = [GCController controllers];
                if(index >= (u32)controllers.count)
                {
                    return ret;
                }
                GCController* controller = controllers[index];
                if(!controller)
                {
                    return ret;
                }
                ret.connected = true;

                if(controller.extendedGamepad)
                {
                    fill_from_extended(controller, controller.extendedGamepad, ret);
                }
                else if(controller.microGamepad)
                {
                    fill_from_micro_gamepad(controller, controller.microGamepad, ret);
                }
            }
            return ret;
        }

        LUNA_HID_API RV set_controller_state(u32 index, const ControllerOutputState& state)
        {
            // Haptics / vibration through GameController is not implemented yet.
            return BasicError::not_supported();
        }
    }
}
