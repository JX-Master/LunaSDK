#include "HID.h"

#include <Luna/HID/HID.hpp>
#include <Luna/HID/Keyboard.hpp>
#include <Luna/HID/Mouse.hpp>
#include <Luna/HID/Controller.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Module.hpp>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

LunaHidPoint2U from_point(const Luna::Int2U& value)
{
    return LunaHidPoint2U{static_cast<uint32_t>(value.x), static_cast<uint32_t>(value.y)};
}

LunaHidControllerInputState from_controller_state(const Luna::HID::ControllerInputState& value)
{
    return LunaHidControllerInputState{
        value.connected ? 1 : 0,
        static_cast<uint32_t>(value.buttons),
        value.axis_lx,
        value.axis_ly,
        value.axis_rx,
        value.axis_ry,
        value.axis_lt,
        value.axis_rt};
}

Luna::HID::ControllerOutputState to_controller_state(const LunaHidControllerOutputState& value)
{
    Luna::HID::ControllerOutputState result;
    result.left_vibration = value.left_vibration;
    result.right_vibration = value.right_vibration;
    return result;
}
}

extern "C"
{
LUNA_HID_C_API luna_errcode_t luna_hid_init_module(void)
{
    Luna::Module* module = Luna::module_hid();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_HID_C_API int32_t luna_hid_supports_keyboard(void)
{
    return Luna::HID::supports_keyboard() ? 1 : 0;
}

LUNA_HID_C_API int32_t luna_hid_get_key_state(uint16_t key)
{
    return Luna::HID::get_key_state(static_cast<Luna::HID::KeyCode>(key)) ? 1 : 0;
}

LUNA_HID_C_API int32_t luna_hid_supports_mouse(void)
{
    return Luna::HID::supports_mouse() ? 1 : 0;
}

LUNA_HID_C_API int32_t luna_hid_get_mouse_button_state(uint8_t mouse_button)
{
    return Luna::HID::get_mouse_button_state(static_cast<Luna::HID::MouseButton>(mouse_button)) ? 1 : 0;
}

LUNA_HID_C_API LunaHidPoint2U luna_hid_get_mouse_pos(void)
{
    return from_point(Luna::HID::get_mouse_pos());
}

LUNA_HID_C_API luna_errcode_t luna_hid_set_mouse_pos(int32_t x, int32_t y)
{
    return from_result(Luna::HID::set_mouse_pos(x, y));
}

LUNA_HID_C_API int32_t luna_hid_supports_controller(void)
{
    return Luna::HID::supports_controller() ? 1 : 0;
}

LUNA_HID_C_API LunaHidControllerInputState luna_hid_get_controller_state(uint32_t index)
{
    return from_controller_state(Luna::HID::get_controller_state(index));
}

LUNA_HID_C_API luna_errcode_t luna_hid_set_controller_state(uint32_t index, const LunaHidControllerOutputState* state)
{
    if (!state)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::HID::set_controller_state(index, to_controller_state(*state)));
}
}
