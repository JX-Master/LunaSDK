namespace Luna.RHI;

public readonly struct RasterizerDesc
{
    public static RasterizerDesc Default => new();

    public RasterizerDesc(
        FillMode fillMode = FillMode.Solid,
        CullMode cullMode = CullMode.Back,
        int depthBias = 0,
        float slopeScaledDepthBias = 0.0f,
        float depthBiasClamp = 0.0f,
        bool frontCounterClockwise = false,
        bool depthClampEnable = false)
    {
        FillMode = fillMode;
        CullMode = cullMode;
        DepthBias = depthBias;
        SlopeScaledDepthBias = slopeScaledDepthBias;
        DepthBiasClamp = depthBiasClamp;
        FrontCounterClockwise = frontCounterClockwise;
        DepthClampEnable = depthClampEnable;
    }

    public int DepthBias { get; }

    public float SlopeScaledDepthBias { get; }

    public float DepthBiasClamp { get; }

    public FillMode FillMode { get; }

    public CullMode CullMode { get; }

    public bool FrontCounterClockwise { get; }

    public bool DepthClampEnable { get; }
}
