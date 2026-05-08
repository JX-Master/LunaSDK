#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_HID_C_API __declspec(dllexport)
#else
#define LUNA_HID_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaHidPoint2U
{
    uint32_t x;
    uint32_t y;
} LunaHidPoint2U;

typedef struct LunaHidControllerInputState
{
    int32_t connected;
    uint32_t buttons;
    float axis_lx;
    float axis_ly;
    float axis_rx;
    float axis_ry;
    float axis_lt;
    float axis_rt;
} LunaHidControllerInputState;

typedef struct LunaHidControllerOutputState
{
    float left_vibration;
    float right_vibration;
} LunaHidControllerOutputState;

LUNA_HID_C_API luna_errcode_t luna_hid_init_module(void);

LUNA_HID_C_API int32_t luna_hid_supports_keyboard(void);
LUNA_HID_C_API int32_t luna_hid_get_key_state(uint16_t key);

LUNA_HID_C_API int32_t luna_hid_supports_mouse(void);
LUNA_HID_C_API int32_t luna_hid_get_mouse_button_state(uint8_t mouse_button);
LUNA_HID_C_API LunaHidPoint2U luna_hid_get_mouse_pos(void);
LUNA_HID_C_API luna_errcode_t luna_hid_set_mouse_pos(int32_t x, int32_t y);

LUNA_HID_C_API int32_t luna_hid_supports_controller(void);
LUNA_HID_C_API LunaHidControllerInputState luna_hid_get_controller_state(uint32_t index);
LUNA_HID_C_API luna_errcode_t luna_hid_set_controller_state(uint32_t index, const LunaHidControllerOutputState* state);

#ifdef __cplusplus
}
#endif
