using System.Runtime.InteropServices;

namespace Luna.HID.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeControllerInputState
{
    public readonly int Connected;
    public readonly uint Buttons;
    public readonly float AxisLX;
    public readonly float AxisLY;
    public readonly float AxisRX;
    public readonly float AxisRY;
    public readonly float AxisLT;
    public readonly float AxisRT;

    public ControllerInputState ToPublic() => new(
        Connected != 0,
        (ControllerButton)Buttons,
        AxisLX,
        AxisLY,
        AxisRX,
        AxisRY,
        AxisLT,
        AxisRT);
}
