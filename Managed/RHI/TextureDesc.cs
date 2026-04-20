namespace Luna.RHI;

public readonly struct TextureDesc
{
    public TextureDesc(
        TextureType type,
        Format format,
        uint width,
        uint height,
        uint depth,
        uint arraySize,
        uint mipLevels,
        uint sampleCount,
        TextureUsageFlags usages,
        ResourceFlags flags)
    {
        Type = type;
        Format = format;
        Width = width;
        Height = height;
        Depth = depth;
        ArraySize = arraySize;
        MipLevels = mipLevels;
        SampleCount = sampleCount;
        Usages = usages;
        Flags = flags;
    }

    public TextureType Type { get; }

    public Format Format { get; }

    public uint Width { get; }

    public uint Height { get; }

    public uint Depth { get; }

    public uint ArraySize { get; }

    public uint MipLevels { get; }

    public uint SampleCount { get; }

    public TextureUsageFlags Usages { get; }

    public ResourceFlags Flags { get; }
}
