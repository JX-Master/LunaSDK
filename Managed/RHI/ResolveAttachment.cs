namespace Luna.RHI;

public readonly struct ResolveAttachment
{
    public ResolveAttachment(ITexture texture, uint mipSlice = 0, uint arraySlice = 0, uint arraySize = 1)
    {
        Texture = texture;
        MipSlice = mipSlice;
        ArraySlice = arraySlice;
        ArraySize = arraySize;
    }

    public ITexture Texture { get; }

    public uint MipSlice { get; }

    public uint ArraySlice { get; }

    public uint ArraySize { get; }
}
