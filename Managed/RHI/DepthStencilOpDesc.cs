namespace Luna.RHI;

public readonly struct DepthStencilOpDesc
{
    public static DepthStencilOpDesc Default => new(
        StencilOp.Keep,
        StencilOp.Keep,
        StencilOp.Keep,
        CompareFunction.Always);

    public DepthStencilOpDesc(
        StencilOp stencilFailOp = StencilOp.Keep,
        StencilOp stencilDepthFailOp = StencilOp.Keep,
        StencilOp stencilPassOp = StencilOp.Keep,
        CompareFunction stencilFunction = CompareFunction.Always)
    {
        StencilFailOp = stencilFailOp;
        StencilDepthFailOp = stencilDepthFailOp;
        StencilPassOp = stencilPassOp;
        StencilFunction = stencilFunction;
    }

    public StencilOp StencilFailOp { get; }

    public StencilOp StencilDepthFailOp { get; }

    public StencilOp StencilPassOp { get; }

    public CompareFunction StencilFunction { get; }
}
