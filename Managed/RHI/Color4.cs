namespace Luna.RHI;

public readonly record struct Color4(float Red, float Green, float Blue, float Alpha)
{
    public static Color4 Black => new(0.0f, 0.0f, 0.0f, 1.0f);

    public static Color4 BlueViolet => new(0.541f, 0.169f, 0.886f, 1.0f);
}
