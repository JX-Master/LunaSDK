using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeViewport
{
    public readonly float TopLeftX;

    public readonly float TopLeftY;

    public readonly float Width;

    public readonly float Height;

    public readonly float MinDepth;

    public readonly float MaxDepth;

    internal NativeViewport(Viewport viewport)
    {
        TopLeftX = viewport.TopLeftX;
        TopLeftY = viewport.TopLeftY;
        Width = viewport.Width;
        Height = viewport.Height;
        MinDepth = viewport.MinDepth;
        MaxDepth = viewport.MaxDepth;
    }
}
