using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;

namespace Luna.Font.Internal;

internal sealed class NativeFontFile : ObjectBase, IFontFile
{
    private readonly IntPtr _ifontFile;

    internal NativeFontFile(NativeFontHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IFontFile == IntPtr.Zero)
        {
            throw new ArgumentException("Native font handle is incomplete.", nameof(handle));
        }
        _ifontFile = handle.IFontFile;
    }

    public byte[] GetData()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.IFontFileGetData(_ifontFile, out var data, out var size)));
        try
        {
            if (size > int.MaxValue)
            {
                throw new InvalidOperationException("The font data is too large to copy into a managed byte array.");
            }
            var result = new byte[(int)size];
            if (size > 0)
            {
                Marshal.Copy(data, result, 0, result.Length);
            }
            return result;
        }
        finally
        {
            RuntimeNative.FreeBuffer(data);
        }
    }

    public uint NumFonts
    {
        get
        {
            EnsureNotDisposed();
            return FontNative.IFontFileGetNumFonts(_ifontFile);
        }
    }

    public int FindGlyph(uint fontIndex, uint codepoint)
    {
        EnsureNotDisposed();
        return FontNative.IFontFileFindGlyph(_ifontFile, fontIndex, codepoint);
    }

    public float ScaleForPixelHeight(uint fontIndex, float pixels)
    {
        EnsureNotDisposed();
        return FontNative.IFontFileScaleForPixelHeight(_ifontFile, fontIndex, pixels);
    }

    public VMetrics GetVMetrics(uint fontIndex)
    {
        EnsureNotDisposed();
        FontNative.IFontFileGetVMetrics(_ifontFile, fontIndex, out var metrics);
        return metrics;
    }

    public GlyphHMetrics GetGlyphHMetrics(uint fontIndex, int glyph)
    {
        EnsureNotDisposed();
        FontNative.IFontFileGetGlyphHMetrics(_ifontFile, fontIndex, glyph, out var metrics);
        return metrics;
    }

    public int GetKernAdvance(uint fontIndex, int glyph1, int glyph2)
    {
        EnsureNotDisposed();
        return FontNative.IFontFileGetKernAdvance(_ifontFile, fontIndex, glyph1, glyph2);
    }

    public short[] GetGlyphShape(uint fontIndex, int glyph)
    {
        EnsureNotDisposed();
        var firstPassCode = new ErrorCode(FontNative.IFontFileGetGlyphShape(_ifontFile, fontIndex, glyph, Array.Empty<short>(), 0, out var count));
        var insufficientBuffer = RuntimeErrors.GetCodeByName("BasicError", "insufficient_user_buffer");
        if (firstPassCode.Failed && firstPassCode != insufficientBuffer)
        {
            RuntimeErrors.ThrowIfFailed(firstPassCode);
        }
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("The glyph shape is too large to copy into a managed array.");
        }
        if (count == 0)
        {
            return Array.Empty<short>();
        }
        var result = new short[(int)count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.IFontFileGetGlyphShape(_ifontFile, fontIndex, glyph, result, count, out count)));
        return result;
    }

    public RectI GetGlyphBoundingBox(uint fontIndex, int glyph)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.IFontFileGetGlyphBoundingBox(_ifontFile, fontIndex, glyph, out var rect)));
        return rect;
    }

    public RectI GetGlyphBitmapBox(uint fontIndex, int glyph, float scaleX, float scaleY, float shiftX, float shiftY)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.IFontFileGetGlyphBitmapBox(_ifontFile, fontIndex, glyph, scaleX, scaleY, shiftX, shiftY, out var rect)));
        return rect;
    }

    public byte[] RenderGlyphBitmap(uint fontIndex, int glyph, int width, int height, int rowPitch, float scaleX, float scaleY, float shiftX, float shiftY)
    {
        EnsureNotDisposed();
        if (width < 0 || height < 0 || rowPitch < width)
        {
            throw new ArgumentOutOfRangeException(nameof(width));
        }
        var bufferSize = checked(rowPitch * height);
        var result = new byte[bufferSize];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNative.IFontFileRenderGlyphBitmap(_ifontFile, fontIndex, glyph, result, width, height, rowPitch, scaleX, scaleY, shiftX, shiftY)));
        return result;
    }
}
