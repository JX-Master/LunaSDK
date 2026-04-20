using System.Runtime.InteropServices;
using Luna.Window;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiRectI
{
    public readonly int OffsetX;

    public readonly int OffsetY;

    public readonly int Width;

    public readonly int Height;

    internal NativeRhiRectI(RectI rect)
    {
        OffsetX = rect.OffsetX;
        OffsetY = rect.OffsetY;
        Width = rect.Width;
        Height = rect.Height;
    }
}
