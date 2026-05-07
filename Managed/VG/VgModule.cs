using System;
using System.Numerics;
using Luna.Font;
using Luna.RHI;
using Luna.Runtime;
using Luna.VG.Internal;

namespace Luna.VG;

public static class VgModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the VG module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.InitModule()));
    }

    public static TextArrangeResult ArrangeText(
        string text,
        TextArrangeSection[] sections,
        RectF boundingRect,
        TextAlignment verticalAlignment,
        TextAlignment horizontalAlignment)
    {
        ArgumentException.ThrowIfNullOrEmpty(text);
        ArgumentNullException.ThrowIfNull(sections);
        var nativeSections = NativeTextArrangeSection.FromPublic(sections);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ArrangeText(
            text,
            ulong.MaxValue,
            nativeSections,
            (ulong)nativeSections.Length,
            in boundingRect,
            (byte)verticalAlignment,
            (byte)horizontalAlignment,
            out var nativeHandle)));
        return LoadTextArrangeResult(new NativeTextArrangeResultHandle(nativeHandle));
    }

    public static (Vertex[] Vertices, uint[] Indices) GenerateTextArrangeResultDrawVertices(
        TextArrangeResult result,
        TextArrangeSection[] sections,
        IFontAtlas fontAtlas)
    {
        ArgumentNullException.ThrowIfNull(result);
        ArgumentNullException.ThrowIfNull(sections);
        ArgumentNullException.ThrowIfNull(fontAtlas);
        var nativeSections = NativeTextArrangeSection.FromPublic(sections);
        var firstPassCode = new ErrorCode(VgNative.GenerateTextArrangeResultDrawVertices(
            result.GetNativeHandle(),
            nativeSections,
            (ulong)nativeSections.Length,
            NativeFontAtlas.GetNativeFontAtlasPointer(fontAtlas),
            null,
            0,
            out var vertexCount,
            null,
            0,
            out var indexCount));
        var insufficientBuffer = RuntimeErrors.GetCodeByName("BasicError", "insufficient_user_buffer");
        if (firstPassCode.Failed && firstPassCode != insufficientBuffer)
        {
            RuntimeErrors.ThrowIfFailed(firstPassCode);
        }
        if (vertexCount > int.MaxValue || indexCount > int.MaxValue)
        {
            throw new InvalidOperationException("The generated draw data is too large to copy into managed arrays.");
        }
        var nativeVertices = new NativeVertex[(int)vertexCount];
        var indices = new uint[(int)indexCount];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.GenerateTextArrangeResultDrawVertices(
            result.GetNativeHandle(),
            nativeSections,
            (ulong)nativeSections.Length,
            NativeFontAtlas.GetNativeFontAtlasPointer(fontAtlas),
            nativeVertices,
            vertexCount,
            out vertexCount,
            indices,
            indexCount,
            out indexCount)));
        var vertices = new Vertex[(int)vertexCount];
        for (var i = 0; i < vertices.Length; ++i)
        {
            vertices[i] = nativeVertices[i].ToPublic();
        }
        return (vertices, indices);
    }

    public static void CommitTextArrangeResult(
        TextArrangeResult result,
        TextArrangeSection[] sections,
        IFontAtlas fontAtlas,
        IShapeDrawList drawList)
    {
        ArgumentNullException.ThrowIfNull(result);
        ArgumentNullException.ThrowIfNull(sections);
        ArgumentNullException.ThrowIfNull(fontAtlas);
        ArgumentNullException.ThrowIfNull(drawList);
        var nativeSections = NativeTextArrangeSection.FromPublic(sections);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.CommitTextArrangeResult(
            result.GetNativeHandle(),
            nativeSections,
            (ulong)nativeSections.Length,
            NativeFontAtlas.GetNativeFontAtlasPointer(fontAtlas),
            NativeShapeDrawList.GetNativeShapeDrawListPointer(drawList))));
    }

    public static IShapeBuffer NewShapeBuffer()
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.NewShapeBuffer(out var shapeBuffer)));
        return new NativeShapeBuffer(shapeBuffer, retain: false);
    }

    public static IFontAtlas NewFontAtlas()
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.NewFontAtlas(out var fontAtlas)));
        return new NativeFontAtlas(fontAtlas, retain: false);
    }

    public static IShapeDrawList NewShapeDrawList(IDevice? device = null)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.NewShapeDrawList(
            device is null ? IntPtr.Zero : RhiDevice.GetNativeDevicePointer(device),
            out var drawList)));
        return new NativeShapeDrawList(drawList, retain: false);
    }

    public static IShapeRenderer NewFillShapeRenderer()
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.NewFillShapeRenderer(out var renderer)));
        return new NativeShapeRenderer(renderer, retain: false);
    }

    public static float[] GetFontGlyphShape(IFontFile fontFile, uint fontIndex, uint codepoint, out RectF boundingRect)
    {
        ArgumentNullException.ThrowIfNull(fontFile);
        var firstPassCode = new ErrorCode(VgNative.GetFontGlyphShape(
            Font.Internal.NativeFontFile.GetNativeFontPointer(fontFile),
            fontIndex,
            codepoint,
            null,
            0,
            out var count,
            out boundingRect));
        var insufficientBuffer = RuntimeErrors.GetCodeByName("BasicError", "insufficient_user_buffer");
        if (firstPassCode.Failed && firstPassCode != insufficientBuffer)
        {
            RuntimeErrors.ThrowIfFailed(firstPassCode);
        }
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("The glyph shape is too large to copy into a managed array.");
        }
        var result = new float[(int)count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.GetFontGlyphShape(
            Font.Internal.NativeFontFile.GetNativeFontPointer(fontFile),
            fontIndex,
            codepoint,
            result,
            count,
            out count,
            out boundingRect)));
        return result;
    }

    public static (Vertex[] Vertices, uint[] Indices) GetRectShapeDrawVertices(
        uint beginCommand,
        uint numCommands,
        Vector2 minPosition,
        Vector2 maxPosition,
        Vector2 minShapeCoord,
        Vector2 maxShapeCoord,
        Vector4? color = null,
        Vector2? minTexCoord = null,
        Vector2? maxTexCoord = null)
    {
        var nativeVertices = new NativeVertex[4];
        var indices = new uint[6];
        var nativeColor = NativeFloat4.FromPublic(color ?? Vector4.One);
        var nativeMinTexCoord = NativeFloat2.FromPublic(minTexCoord ?? Vector2.Zero);
        var nativeMaxTexCoord = NativeFloat2.FromPublic(maxTexCoord ?? Vector2.Zero);
        var nativeMinPosition = NativeFloat2.FromPublic(minPosition);
        var nativeMaxPosition = NativeFloat2.FromPublic(maxPosition);
        var nativeMinShapeCoord = NativeFloat2.FromPublic(minShapeCoord);
        var nativeMaxShapeCoord = NativeFloat2.FromPublic(maxShapeCoord);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.GetRectShapeDrawVertices(
            nativeVertices,
            indices,
            beginCommand,
            numCommands,
            in nativeMinPosition,
            in nativeMaxPosition,
            in nativeMinShapeCoord,
            in nativeMaxShapeCoord,
            in nativeColor,
            in nativeMinTexCoord,
            in nativeMaxTexCoord)));
        var vertices = new Vertex[4];
        for (var i = 0; i < 4; ++i)
        {
            vertices[i] = nativeVertices[i].ToPublic();
        }
        return (vertices, indices);
    }

    private static TextArrangeResult LoadTextArrangeResult(NativeTextArrangeResultHandle handle)
    {
        try
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.TextArrangeResultGetBoundingRect(handle.DangerousGetHandle(), out var boundingRect)));
            var overflow = VgNative.TextArrangeResultGetOverflow(handle.DangerousGetHandle()) != 0;
            var lineCount = VgNative.TextArrangeResultGetNumLines(handle.DangerousGetHandle());
            if (lineCount > int.MaxValue)
            {
                throw new InvalidOperationException("The arranged text has too many lines to copy into a managed array.");
            }
            var lines = new TextLineArrangeResult[(int)lineCount];
            for (ulong lineIndex = 0; lineIndex < lineCount; ++lineIndex)
            {
                RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.TextArrangeResultGetLine(handle.DangerousGetHandle(), lineIndex, out var line)));
                var glyphCount = VgNative.TextArrangeResultGetNumGlyphs(handle.DangerousGetHandle(), lineIndex);
                if (glyphCount > int.MaxValue)
                {
                    throw new InvalidOperationException("The arranged text line has too many glyphs to copy into a managed array.");
                }
                var glyphs = new TextGlyphArrangeResult[(int)glyphCount];
                for (ulong glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex)
                {
                    RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.TextArrangeResultGetGlyph(handle.DangerousGetHandle(), lineIndex, glyphIndex, out var glyph)));
                    glyphs[(int)glyphIndex] = glyph.ToPublic();
                }
                lines[(int)lineIndex] = line.ToPublic(glyphs);
            }
            return new TextArrangeResult(handle, boundingRect, overflow, lines);
        }
        catch
        {
            handle.Dispose();
            throw;
        }
    }
}
