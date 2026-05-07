using Luna.Runtime;

namespace Luna.Font;

public interface IFontFile : IObject
{
    byte[] GetData();

    uint NumFonts { get; }

    int FindGlyph(uint fontIndex, uint codepoint);

    float ScaleForPixelHeight(uint fontIndex, float pixels);

    VMetrics GetVMetrics(uint fontIndex);

    GlyphHMetrics GetGlyphHMetrics(uint fontIndex, int glyph);

    int GetKernAdvance(uint fontIndex, int glyph1, int glyph2);

    short[] GetGlyphShape(uint fontIndex, int glyph);

    RectI GetGlyphBoundingBox(uint fontIndex, int glyph);

    RectI GetGlyphBitmapBox(uint fontIndex, int glyph, float scaleX, float scaleY, float shiftX, float shiftY);

    byte[] RenderGlyphBitmap(uint fontIndex, int glyph, int width, int height, int rowPitch, float scaleX, float scaleY, float shiftX, float shiftY);
}
