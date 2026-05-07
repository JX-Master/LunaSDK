namespace Luna.VG;

public sealed class TextLineArrangeResult
{
    internal TextLineArrangeResult(RectF boundingRect, float baselineOffset, float ascent, float decent, float lineGap, TextGlyphArrangeResult[] glyphs)
    {
        BoundingRect = boundingRect;
        BaselineOffset = baselineOffset;
        Ascent = ascent;
        Decent = decent;
        LineGap = lineGap;
        Glyphs = glyphs;
    }

    public RectF BoundingRect { get; }
    public float BaselineOffset { get; }
    public float Ascent { get; }
    public float Decent { get; }
    public float LineGap { get; }
    public TextGlyphArrangeResult[] Glyphs { get; }
}
