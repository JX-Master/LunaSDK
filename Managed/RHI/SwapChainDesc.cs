namespace Luna.RHI;

public readonly struct SwapChainDesc
{
    public SwapChainDesc(
        uint width,
        uint height,
        uint bufferCount,
        Format format,
        bool verticalSynchronized,
        ColorSpace colorSpace = ColorSpace.Unspecified)
    {
        Width = width;
        Height = height;
        BufferCount = bufferCount;
        Format = format;
        VerticalSynchronized = verticalSynchronized;
        ColorSpace = colorSpace;
    }

    public uint Width { get; }

    public uint Height { get; }

    public uint BufferCount { get; }

    public Format Format { get; }

    public ColorSpace ColorSpace { get; }

    public bool VerticalSynchronized { get; }
}
