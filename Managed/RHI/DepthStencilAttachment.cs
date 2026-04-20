namespace Luna.RHI;

public readonly struct DepthStencilAttachment
{
    public DepthStencilAttachment(
        ITexture texture,
        bool readOnly,
        LoadOp depthLoadOp = LoadOp.Load,
        StoreOp depthStoreOp = StoreOp.Store,
        float depthClearValue = 1.0f,
        LoadOp stencilLoadOp = LoadOp.DontCare,
        StoreOp stencilStoreOp = StoreOp.DontCare,
        byte stencilClearValue = 0,
        TextureViewType viewType = TextureViewType.Unspecified,
        Format format = Format.Unknown,
        uint mipSlice = 0,
        uint arraySlice = 0)
    {
        Texture = texture;
        ReadOnly = readOnly;
        DepthLoadOp = depthLoadOp;
        DepthStoreOp = depthStoreOp;
        DepthClearValue = depthClearValue;
        StencilLoadOp = stencilLoadOp;
        StencilStoreOp = stencilStoreOp;
        StencilClearValue = stencilClearValue;
        ViewType = viewType;
        Format = format;
        MipSlice = mipSlice;
        ArraySlice = arraySlice;
    }

    public ITexture Texture { get; }

    public bool ReadOnly { get; }

    public LoadOp DepthLoadOp { get; }

    public StoreOp DepthStoreOp { get; }

    public float DepthClearValue { get; }

    public LoadOp StencilLoadOp { get; }

    public StoreOp StencilStoreOp { get; }

    public byte StencilClearValue { get; }

    public TextureViewType ViewType { get; }

    public Format Format { get; }

    public uint MipSlice { get; }

    public uint ArraySlice { get; }
}
