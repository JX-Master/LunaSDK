using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeSwapChainDesc
{
    public readonly uint Width;
    public readonly uint Height;
    public readonly uint BufferCount;
    public readonly uint Format;
    public readonly uint ColorSpace;
    public readonly int VerticalSynchronized;

    private NativeSwapChainDesc(uint width, uint height, uint bufferCount, uint format, uint colorSpace, int verticalSynchronized)
    {
        Width = width;
        Height = height;
        BufferCount = bufferCount;
        Format = format;
        ColorSpace = colorSpace;
        VerticalSynchronized = verticalSynchronized;
    }

    internal static NativeSwapChainDesc FromPublic(SwapChainDesc desc)
    {
        return new NativeSwapChainDesc(
            desc.Width,
            desc.Height,
            desc.BufferCount,
            (uint)desc.Format,
            (uint)desc.ColorSpace,
            desc.VerticalSynchronized ? 1 : 0);
    }

    internal SwapChainDesc ToPublic()
    {
        return new SwapChainDesc(
            Width,
            Height,
            BufferCount,
            (Format)Format,
            VerticalSynchronized != 0,
            (ColorSpace)ColorSpace);
    }
}
