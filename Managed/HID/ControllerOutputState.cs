using System.Runtime.InteropServices;

namespace Luna.HID;

[StructLayout(LayoutKind.Sequential)]
public readonly struct ControllerOutputState
{
    public ControllerOutputState(float leftVibration, float rightVibration)
    {
        LeftVibration = leftVibration;
        RightVibration = rightVibration;
    }

    public readonly float LeftVibration;

    public readonly float RightVibration;
}
