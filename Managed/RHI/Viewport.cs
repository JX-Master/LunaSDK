namespace Luna.RHI;

public readonly struct Viewport
{
    public Viewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
    {
        TopLeftX = topLeftX;
        TopLeftY = topLeftY;
        Width = width;
        Height = height;
        MinDepth = minDepth;
        MaxDepth = maxDepth;
    }

    public float TopLeftX { get; }

    public float TopLeftY { get; }

    public float Width { get; }

    public float Height { get; }

    public float MinDepth { get; }

    public float MaxDepth { get; }
}
