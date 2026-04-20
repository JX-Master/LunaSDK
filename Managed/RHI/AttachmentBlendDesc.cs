namespace Luna.RHI;

public readonly struct AttachmentBlendDesc
{
    public static AttachmentBlendDesc Default => new(
        blendEnable: false,
        sourceBlendColor: BlendFactor.One,
        destinationBlendColor: BlendFactor.Zero,
        blendOpColor: BlendOp.Add,
        sourceBlendAlpha: BlendFactor.One,
        destinationBlendAlpha: BlendFactor.Zero,
        blendOpAlpha: BlendOp.Add,
        colorWriteMask: ColorWriteMask.All);

    public AttachmentBlendDesc(
        bool blendEnable = false,
        BlendFactor sourceBlendColor = BlendFactor.One,
        BlendFactor destinationBlendColor = BlendFactor.Zero,
        BlendOp blendOpColor = BlendOp.Add,
        BlendFactor sourceBlendAlpha = BlendFactor.One,
        BlendFactor destinationBlendAlpha = BlendFactor.Zero,
        BlendOp blendOpAlpha = BlendOp.Add,
        ColorWriteMask colorWriteMask = ColorWriteMask.All)
    {
        BlendEnable = blendEnable;
        SourceBlendColor = sourceBlendColor;
        DestinationBlendColor = destinationBlendColor;
        BlendOpColor = blendOpColor;
        SourceBlendAlpha = sourceBlendAlpha;
        DestinationBlendAlpha = destinationBlendAlpha;
        BlendOpAlpha = blendOpAlpha;
        ColorWriteMask = colorWriteMask;
    }

    public bool BlendEnable { get; }

    public BlendFactor SourceBlendColor { get; }

    public BlendFactor DestinationBlendColor { get; }

    public BlendOp BlendOpColor { get; }

    public BlendFactor SourceBlendAlpha { get; }

    public BlendFactor DestinationBlendAlpha { get; }

    public BlendOp BlendOpAlpha { get; }

    public ColorWriteMask ColorWriteMask { get; }
}
