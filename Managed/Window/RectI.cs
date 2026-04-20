using System.Runtime.InteropServices;

namespace Luna.Window;

[StructLayout(LayoutKind.Sequential)]
public readonly struct RectI
{
    public RectI(int offsetX, int offsetY, int width, int height)
    {
        OffsetX = offsetX;
        OffsetY = offsetY;
        Width = width;
        Height = height;
    }

    public readonly int OffsetX;

    public readonly int OffsetY;

    public readonly int Width;

    public readonly int Height;

    public override string ToString()
    {
        return $"{OffsetX},{OffsetY} {Width}x{Height}";
    }
}
