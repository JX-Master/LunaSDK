using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeClearValue
{
    public readonly uint Format;
    public readonly uint Type;
    public readonly float ColorRed;
    public readonly float ColorGreen;
    public readonly float ColorBlue;
    public readonly float ColorAlpha;
    public readonly float Depth;
    public readonly byte Stencil;

    public NativeClearValue(ClearValue value)
    {
        if (value.Type != ClearValueType.Color && value.Type != ClearValueType.DepthStencil)
        {
            throw new ArgumentException("The optimized clear value type is invalid.", nameof(value));
        }

        Format = (uint)value.Format;
        Type = (uint)value.Type;
        ColorRed = value.Color.Red;
        ColorGreen = value.Color.Green;
        ColorBlue = value.Color.Blue;
        ColorAlpha = value.Color.Alpha;
        Depth = value.Depth;
        Stencil = value.Stencil;
    }
}
