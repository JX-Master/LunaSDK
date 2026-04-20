namespace Luna.RHI;

public readonly struct SamplerDesc
{
    public SamplerDesc(
        Filter minFilter,
        Filter magFilter,
        Filter mipFilter,
        TextureAddressMode addressU,
        TextureAddressMode addressV,
        TextureAddressMode addressW,
        bool anisotropyEnable = false,
        uint maxAnisotropy = 1,
        BorderColor borderColor = BorderColor.Float0000,
        float minLod = 0.0f,
        float maxLod = float.MaxValue,
        bool compareEnable = false,
        CompareFunction compareFunction = CompareFunction.Always)
    {
        MinFilter = minFilter;
        MagFilter = magFilter;
        MipFilter = mipFilter;
        AddressU = addressU;
        AddressV = addressV;
        AddressW = addressW;
        AnisotropyEnable = anisotropyEnable;
        MaxAnisotropy = maxAnisotropy;
        BorderColor = borderColor;
        MinLod = minLod;
        MaxLod = maxLod;
        CompareEnable = compareEnable;
        CompareFunction = compareFunction;
    }

    public Filter MinFilter { get; }
    public Filter MagFilter { get; }
    public Filter MipFilter { get; }
    public TextureAddressMode AddressU { get; }
    public TextureAddressMode AddressV { get; }
    public TextureAddressMode AddressW { get; }
    public bool AnisotropyEnable { get; }
    public bool CompareEnable { get; }
    public CompareFunction CompareFunction { get; }
    public BorderColor BorderColor { get; }
    public uint MaxAnisotropy { get; }
    public float MinLod { get; }
    public float MaxLod { get; }
}
