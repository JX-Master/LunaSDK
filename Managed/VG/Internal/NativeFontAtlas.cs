using System;
using Luna.Font;
using Luna.Font.Internal;
using Luna.Runtime;

namespace Luna.VG.Internal;

internal sealed class NativeFontAtlas : ObjectBase, IFontAtlas
{
    private readonly IntPtr _fontAtlas;

    internal NativeFontAtlas(NativeFontAtlasHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IFontAtlas == IntPtr.Zero)
        {
            throw new ArgumentException("Native font atlas handle is incomplete.", nameof(handle));
        }
        _fontAtlas = handle.IFontAtlas;
    }

    public void Clear()
    {
        EnsureNotDisposed();
        VgNative.FontAtlasClear(_fontAtlas);
    }

    public IFontFile? GetFont(out uint index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.FontAtlasGetFont(_fontAtlas, out var font, out index)));
        return font.Object == IntPtr.Zero ? null : new NativeFontFile(font, retain: false);
    }

    public void SetFont(IFontFile fontFile, uint index)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(fontFile);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.FontAtlasSetFont(_fontAtlas, NativeFontFile.GetNativeFontPointer(fontFile), index)));
    }

    public IShapeBuffer GetShapeBuffer()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.FontAtlasGetShapeBuffer(_fontAtlas, out var shapeBuffer)));
        return new NativeShapeBuffer(shapeBuffer, retain: false);
    }

    public FontAtlasGlyph GetGlyph(uint codepoint)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.FontAtlasGetGlyph(_fontAtlas, codepoint, out var firstShapePoint, out var numShapePoints, out var boundingRect)));
        return new FontAtlasGlyph(firstShapePoint, numShapePoints, boundingRect);
    }

    internal static IntPtr GetNativeFontAtlasPointer(IFontAtlas fontAtlas)
    {
        ArgumentNullException.ThrowIfNull(fontAtlas);
        if (fontAtlas is not NativeFontAtlas nativeFontAtlas)
        {
            throw new ArgumentException("The font atlas must be created by Luna.VG.", nameof(fontAtlas));
        }
        nativeFontAtlas.EnsureNotDisposed();
        return nativeFontAtlas._fontAtlas;
    }
}
