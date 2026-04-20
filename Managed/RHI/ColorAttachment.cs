namespace Luna.RHI;

public readonly struct ColorAttachment
{
    public ColorAttachment(
        ITexture texture,
        LoadOp loadOp = LoadOp.Load,
        StoreOp storeOp = StoreOp.Store,
        Color4 clearValue = default,
        TextureViewType viewType = TextureViewType.Unspecified,
        Format format = Format.Unknown,
        uint mipSlice = 0,
        uint arraySlice = 0)
    {
        Texture = texture;
        LoadOp = loadOp;
        StoreOp = storeOp;
        ClearValue = clearValue;
        ViewType = viewType;
        Format = format;
        MipSlice = mipSlice;
        ArraySlice = arraySlice;
    }

    public ITexture Texture { get; }

    public LoadOp LoadOp { get; }

    public StoreOp StoreOp { get; }

    public Color4 ClearValue { get; }

    public TextureViewType ViewType { get; }

    public Format Format { get; }

    public uint MipSlice { get; }

    public uint ArraySlice { get; }
}
