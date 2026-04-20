namespace Luna.RHI;

public readonly struct ClearValue
{
    private ClearValue(Format format, ClearValueType type, Color4 color, float depth, byte stencil)
    {
        Format = format;
        Type = type;
        Color = color;
        Depth = depth;
        Stencil = stencil;
    }

    public Format Format { get; }

    public ClearValueType Type { get; }

    public Color4 Color { get; }

    public float Depth { get; }

    public byte Stencil { get; }

    public static ClearValue ForColor(Format format, Color4 color)
    {
        return new ClearValue(format, ClearValueType.Color, color, 0.0f, 0);
    }

    public static ClearValue ForDepthStencil(Format format, float depth, byte stencil = 0)
    {
        return new ClearValue(format, ClearValueType.DepthStencil, Color4.Black, depth, stencil);
    }
}
