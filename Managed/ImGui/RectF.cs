using System.Runtime.InteropServices;

namespace Luna.ImGui;

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

    public float OffsetX { get; }

    public float OffsetY { get; }

    public float Width { get; }

    public float Height { get; }
}
