using System;
using Luna.Font;
using Luna.Runtime;

Runtime.Init();

try
{
    FontModule.Init();

    using var defaultFont = FontModule.GetDefaultFont();
    if (defaultFont.NumFonts == 0)
    {
        throw new InvalidOperationException("Default font should contain at least one font face.");
    }

    const uint fontIndex = 0;
    var glyphA = defaultFont.FindGlyph(fontIndex, 'A');
    if (glyphA == FontModule.InvalidGlyph)
    {
        throw new InvalidOperationException("Default font should contain glyph A.");
    }

    var scale = defaultFont.ScaleForPixelHeight(fontIndex, 32.0f);
    if (scale <= 0.0f)
    {
        throw new InvalidOperationException("Font scale should be positive.");
    }

    var vmetrics = defaultFont.GetVMetrics(fontIndex);
    if (vmetrics.Ascent <= 0 || vmetrics.Descent >= 0)
    {
        throw new InvalidOperationException("Unexpected default font vertical metrics.");
    }

    var hmetrics = defaultFont.GetGlyphHMetrics(fontIndex, glyphA);
    if (hmetrics.AdvanceWidth <= 0)
    {
        throw new InvalidOperationException("Glyph advance width should be positive.");
    }

    var glyphShape = defaultFont.GetGlyphShape(fontIndex, glyphA);
    if (glyphShape.Length == 0)
    {
        throw new InvalidOperationException("Glyph shape should not be empty.");
    }

    var glyphBox = defaultFont.GetGlyphBoundingBox(fontIndex, glyphA);
    if (glyphBox.Width <= 0 || glyphBox.Height <= 0)
    {
        throw new InvalidOperationException("Glyph bounding box should be positive.");
    }

    var bitmapBox = defaultFont.GetGlyphBitmapBox(fontIndex, glyphA, scale, scale, 0.0f, 0.0f);
    if (bitmapBox.Width <= 0 || bitmapBox.Height <= 0)
    {
        throw new InvalidOperationException("Glyph bitmap box should be positive.");
    }

    var bitmap = defaultFont.RenderGlyphBitmap(fontIndex, glyphA, bitmapBox.Width, bitmapBox.Height, bitmapBox.Width, scale, scale, 0.0f, 0.0f);
    if (Array.TrueForAll(bitmap, value => value == 0))
    {
        throw new InvalidOperationException("Glyph bitmap should contain non-zero coverage.");
    }

    var glyphV = defaultFont.FindGlyph(fontIndex, 'V');
    var kern = defaultFont.GetKernAdvance(fontIndex, glyphA, glyphV);
    if (kern == int.MinValue)
    {
        throw new InvalidOperationException("Glyph kern advance returned an unexpected value.");
    }

    var fontData = defaultFont.GetData();
    if (fontData.Length == 0)
    {
        throw new InvalidOperationException("Default font data should not be empty.");
    }

    using var copiedFont = FontModule.LoadTtfFontFile(fontData);
    if (copiedFont.NumFonts != defaultFont.NumFonts)
    {
        throw new InvalidOperationException("Loaded font face count mismatch.");
    }
    if (copiedFont.FindGlyph(fontIndex, 'A') != glyphA)
    {
        throw new InvalidOperationException("Loaded font glyph lookup mismatch.");
    }
    if (Math.Abs(copiedFont.ScaleForPixelHeight(fontIndex, 32.0f) - scale) > 0.0001f)
    {
        throw new InvalidOperationException("Loaded font scale mismatch.");
    }

    Console.WriteLine("FontCSharpTest passed.");
}
finally
{
    Runtime.Close();
}
