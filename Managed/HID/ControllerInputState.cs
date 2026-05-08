namespace Luna.HID;

public readonly record struct ControllerInputState(
    bool Connected,
    ControllerButton Buttons,
    float AxisLX,
    float AxisLY,
    float AxisRX,
    float AxisRY,
    float AxisLT,
    float AxisRT);
