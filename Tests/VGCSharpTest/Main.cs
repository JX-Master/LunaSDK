using System;
using System.Linq;
using System.Numerics;
using Luna.Font;
using Luna.RHI;
using Luna.Runtime;
using Luna.VG;

static class Program
{
    static void Main()
    {
        Runtime.Init();
        try
        {
            FontModule.Init();
            var badPlatformCall = RuntimeErrors.GetCodeByName("BasicError", "bad_platform_call");
            var rhiAvailable = false;
            try
            {
                Module.Init();
                VgModule.Init();
                rhiAvailable = true;
            }
            catch (ErrorException ex) when (new ErrorCode(ex.Code) == badPlatformCall)
            {
                Console.WriteLine("Skipping RHI-backed VG checks: bad_platform_call.");
            }

            Console.WriteLine("cpu:font");
            using var font = FontModule.GetDefaultFont();
            var sections = new[]
            {
                new TextArrangeSection
                {
                    FontFile = font,
                    NumChars = 15,
                    FontIndex = 0,
                    FontSize = 48.0f,
                    Color = Vector4.One
                }
            };

            Console.WriteLine("cpu:arrange");
            using var arranged = VgModule.ArrangeText(
                "Vector Graphics",
                sections,
                new RectF(0.0f, 0.0f, 800.0f, 300.0f),
                TextAlignment.Begin,
                TextAlignment.Center);
            if (arranged.Overflow)
            {
                throw new Exception("Text arrangement unexpectedly overflowed.");
            }
            if (arranged.Lines.Length == 0 || arranged.Lines[0].Glyphs.Length == 0)
            {
                throw new Exception("Text arrangement did not produce glyphs.");
            }

            Console.WriteLine("cpu:glyph");
            var glyphShape = VgModule.GetFontGlyphShape(font, 0, (uint)'V', out var glyphBounds);
            if (glyphShape.Length == 0 || glyphBounds.Width <= 0.0f || glyphBounds.Height <= 0.0f)
            {
                throw new Exception("Glyph shape query failed.");
            }

            var (generatedRectVertices, generatedRectIndices) = VgModule.GetRectShapeDrawVertices(
                0,
                (uint)glyphShape.Length,
                Vector2.Zero,
                new Vector2(glyphBounds.Width, glyphBounds.Height),
                Vector2.Zero,
                new Vector2(glyphBounds.Width, glyphBounds.Height));
            if (generatedRectVertices.Length != 4 || generatedRectIndices.Length != 6)
            {
                throw new Exception("Rectangle draw vertex helper returned unexpected sizes.");
            }
            Console.WriteLine("cpu:done");

            if (rhiAvailable)
            {
                var device = Module.GetMainDevice();
                using (device)
                {
                    Console.WriteLine("gpu:atlas");
                    using var atlas = VgModule.NewFontAtlas();
                    atlas.SetFont(font, 0);
                    var queriedFont = atlas.GetFont(out var queriedFontIndex);
                    if (queriedFont is null || queriedFontIndex != 0)
                    {
                        throw new Exception("Font atlas did not preserve the bound font.");
                    }
                    queriedFont.Dispose();

                    var glyph = atlas.GetGlyph((uint)'V');
                    if (glyph.NumShapePoints == 0 || glyph.BoundingRect.Width <= 0.0f || glyph.BoundingRect.Height <= 0.0f)
                    {
                        throw new Exception("Font atlas did not resolve a valid glyph.");
                    }

                    Console.WriteLine("gpu:shape-buffer");
                    using var shapeBuffer = VgModule.NewShapeBuffer();
                    shapeBuffer.SetShapePoints(glyphShape);
                    var roundtripShape = shapeBuffer.GetShapePoints();
                    if (!glyphShape.SequenceEqual(roundtripShape))
                    {
                        throw new Exception("Shape buffer point roundtrip failed.");
                    }

                    var (generatedVertices, generatedIndices) = VgModule.GenerateTextArrangeResultDrawVertices(arranged, sections, atlas);
                    if (generatedVertices.Length == 0 || generatedIndices.Length == 0)
                    {
                        throw new Exception("Generated text draw data is empty.");
                    }

                    using var builtShapeBuffer = shapeBuffer.Build(device);
                    if (builtShapeBuffer is null)
                    {
                        throw new Exception("Shape buffer build returned null.");
                    }

                    using var drawList = VgModule.NewShapeDrawList(device);
                    drawList.Reset();
                    drawList.SetShapeBuffer(shapeBuffer);
                    drawList.SetClipRect(new RectF(10.0f, 20.0f, 200.0f, 100.0f));
                    var clipRect = drawList.GetClipRect();
                    if (clipRect.Width != 200.0f || clipRect.Height != 100.0f)
                    {
                        throw new Exception("Shape draw list clip rect roundtrip failed.");
                    }
                    drawList.SetTransform(Matrix4x4.Identity);
                    VgModule.CommitTextArrangeResult(arranged, sections, atlas, drawList);
                    drawList.Compile();
                    if (drawList.GetVertexBufferSize() == 0 || drawList.GetIndexBufferSize() == 0)
                    {
                        throw new Exception("Shape draw list compile produced empty buffers.");
                    }
                    using var vertexBuffer = drawList.GetVertexBuffer();
                    using var indexBuffer = drawList.GetIndexBuffer();
                    if (vertexBuffer is null || indexBuffer is null)
                    {
                        throw new Exception("Shape draw list compile did not produce native buffers.");
                    }
                    var drawCalls = drawList.GetDrawCalls();
                    if (drawCalls.Length == 0)
                    {
                        throw new Exception("Shape draw list compile did not produce draw calls.");
                    }
                    if (drawCalls[0].ShapeBuffer is null)
                    {
                        throw new Exception("Compiled draw call is missing its shape buffer.");
                    }

                    var (rectVertices, rectIndices) = VgModule.GetRectShapeDrawVertices(
                        (uint)glyph.FirstShapePoint,
                        (uint)glyph.NumShapePoints,
                        new Vector2(0.0f, 0.0f),
                        new Vector2(glyph.BoundingRect.Width, glyph.BoundingRect.Height),
                        Vector2.Zero,
                        new Vector2(glyph.BoundingRect.Width, glyph.BoundingRect.Height));
                    if (rectVertices.Length != 4 || rectIndices.Length != 6)
                    {
                        throw new Exception("Rectangle draw vertex helper returned unexpected sizes.");
                    }

                    drawList.Reset();
                    drawList.SetShapeBuffer(shapeBuffer);
                    drawList.DrawShapeRaw(rectVertices, rectIndices);
                    drawList.Compile();
                    if (drawList.GetDrawCalls().Length == 0)
                    {
                        throw new Exception("DrawShapeRaw path did not produce draw calls.");
                    }
                }
            }

            Console.WriteLine("VGCSharpTest passed.");
        }
        finally
        {
            Runtime.Close();
        }
    }
}
