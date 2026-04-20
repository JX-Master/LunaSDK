using System.Runtime.InteropServices;

namespace Luna.Window;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Point2I
{
    public Point2I(int x, int y)
    {
        X = x;
        Y = y;
    }

    public readonly int X;

    public readonly int Y;

    public override string ToString()
    {
        return $"{X},{Y}";
    }
}
