namespace Luna.RHI;

public readonly struct TextureViewDesc
{
    public static TextureViewDesc Tex1D(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex1D, format, mipSlice, mipSize, 0, 1);
    }

    public static TextureViewDesc Tex1DArray(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue, uint arraySlice = 0, uint arraySize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex1DArray, format, mipSlice, mipSize, arraySlice, arraySize);
    }

    public static TextureViewDesc Tex2D(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex2D, format, mipSlice, mipSize, 0, 1);
    }

    public static TextureViewDesc Tex2DArray(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue, uint arraySlice = 0, uint arraySize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex2DArray, format, mipSlice, mipSize, arraySlice, arraySize);
    }

    public static TextureViewDesc Tex2DMultisample(ITexture texture, Format format = Format.Unknown)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex2DMs, format, 0, 1, 0, 1);
    }

    public static TextureViewDesc Tex2DMultisampleArray(ITexture texture, Format format = Format.Unknown, uint arraySlice = 0, uint arraySize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex2DMsArray, format, 0, 1, arraySlice, arraySize);
    }

    public static TextureViewDesc Tex3D(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.Tex3D, format, mipSlice, mipSize, 0, 1);
    }

    public static TextureViewDesc TexCube(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.TexCube, format, mipSlice, mipSize, 0, 6);
    }

    public static TextureViewDesc TexCubeArray(ITexture texture, Format format = Format.Unknown, uint mipSlice = 0, uint mipSize = uint.MaxValue, uint arraySlice = 0, uint arraySize = uint.MaxValue)
    {
        return new TextureViewDesc(texture, TextureViewType.TexCubeArray, format, mipSlice, mipSize, arraySlice, arraySize);
    }

    public TextureViewDesc(ITexture texture, TextureViewType type, Format format, uint mipSlice, uint mipSize, uint arraySlice, uint arraySize)
    {
        Texture = texture;
        Type = type;
        Format = format;
        MipSlice = mipSlice;
        MipSize = mipSize;
        ArraySlice = arraySlice;
        ArraySize = arraySize;
    }

    public ITexture Texture { get; }

    public TextureViewType Type { get; }

    public Format Format { get; }

    public uint MipSlice { get; }

    public uint MipSize { get; }

    public uint ArraySlice { get; }

    public uint ArraySize { get; }
}
