using System.Numerics;
using Luna.Font;

namespace Luna.VG;

public sealed class TextArrangeSection
{
    public IFontFile FontFile { get; set; } = null!;

    public ulong NumChars { get; set; }

    public uint FontIndex { get; set; }

    public Vector4 Color { get; set; } = Vector4.One;

    public float FontSize { get; set; } = 18.0f;

    public float CharSpan { get; set; }

    public float LineSpan { get; set; }
}
