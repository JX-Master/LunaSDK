namespace Luna.HID.Internal;

internal partial struct NativeControllerInputState
{
    public ControllerInputState ToPublic() => new(
        Connected != 0,
        (ControllerButton)Buttons,
        AxisLx,
        AxisLy,
        AxisRx,
        AxisRy,
        AxisLt,
        AxisRt);
}
