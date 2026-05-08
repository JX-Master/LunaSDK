using System.Runtime.InteropServices;

namespace Luna.HID;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Point2U
{
    public Point2U(uint x, uint y)
    {
        X = x;
        Y = y;
    }

    public readonly uint X;

    public readonly uint Y;
}
