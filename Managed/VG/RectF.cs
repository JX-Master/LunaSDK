using System.Runtime.InteropServices;

namespace Luna.VG;

[StructLayout(LayoutKind.Sequential)]
public readonly struct RectF
{
    public RectF(float offsetX, float offsetY, float width, float height)
    {
        OffsetX = offsetX;
        OffsetY = offsetY;
        Width = width;
        Height = height;
    }

    public readonly float OffsetX;
    public readonly float OffsetY;
    public readonly float Width;
    public readonly float Height;
}
