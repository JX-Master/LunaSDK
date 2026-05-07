namespace Luna.RHI;

public readonly struct DepthStencilDesc
{
    public static DepthStencilDesc Default => new(
        depthTestEnable: true,
        depthWriteEnable: true,
        depthFunction: CompareFunction.Less,
        stencilEnable: false,
        stencilReadMask: 0xff,
        stencilWriteMask: 0xff,
        frontFace: DepthStencilOpDesc.Default,
        backFace: DepthStencilOpDesc.Default);

    public DepthStencilDesc(
        bool depthTestEnable = true,
        bool depthWriteEnable = true,
        CompareFunction depthFunction = CompareFunction.Less,
        bool stencilEnable = false,
        byte stencilReadMask = 0xff,
        byte stencilWriteMask = 0xff,
        DepthStencilOpDesc? frontFace = null,
        DepthStencilOpDesc? backFace = null)
    {
        DepthTestEnable = depthTestEnable;
        DepthWriteEnable = depthWriteEnable;
        DepthFunction = depthFunction;
        StencilEnable = stencilEnable;
        StencilReadMask = stencilReadMask;
        StencilWriteMask = stencilWriteMask;
        FrontFace = frontFace ?? DepthStencilOpDesc.Default;
        BackFace = backFace ?? DepthStencilOpDesc.Default;
    }

    public bool DepthTestEnable { get; }

    public bool DepthWriteEnable { get; }

    public CompareFunction DepthFunction { get; }

    public bool StencilEnable { get; }

    public byte StencilReadMask { get; }

    public byte StencilWriteMask { get; }

    public DepthStencilOpDesc FrontFace { get; }

    public DepthStencilOpDesc BackFace { get; }
}
