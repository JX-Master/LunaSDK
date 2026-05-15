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
        if (handle.IfontFile == IntPtr.Zero)
        {
            throw new ArgumentException("Native font handle is incomplete.", nameof(handle));
        }
        _ifontFile = handle.IfontFile;
    }

    public byte[] GetData()
    {
        EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.IfontFileGetData(_ifontFile, out var data, out var size)));
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
            RuntimeNativeGenerated.FreeBuffer(data);
        }
    }

    public uint NumFonts
    {
        get
        {
            EnsureNotDisposed();
            return FontNativeGenerated.IfontFileGetNumFonts(_ifontFile);
        }
    }

    public int FindGlyph(uint fontIndex, uint codepoint)
    {
        EnsureNotDisposed();
        return FontNativeGenerated.IfontFileFindGlyph(_ifontFile, fontIndex, codepoint);
    }

    public float ScaleForPixelHeight(uint fontIndex, float pixels)
    {
        EnsureNotDisposed();
        return FontNativeGenerated.IfontFileScaleForPixelHeight(_ifontFile, fontIndex, pixels);
    }

    public VMetrics GetVMetrics(uint fontIndex)
    {
        EnsureNotDisposed();
        FontNativeGenerated.IfontFileGetVmetrics(_ifontFile, fontIndex, out var metrics);
        return metrics;
    }

    public GlyphHMetrics GetGlyphHMetrics(uint fontIndex, int glyph)
    {
        EnsureNotDisposed();
        FontNativeGenerated.IfontFileGetGlyphHmetrics(_ifontFile, fontIndex, glyph, out var metrics);
        return metrics;
    }

    public int GetKernAdvance(uint fontIndex, int glyph1, int glyph2)
    {
        EnsureNotDisposed();
        return FontNativeGenerated.IfontFileGetKernAdvance(_ifontFile, fontIndex, glyph1, glyph2);
    }

    public short[] GetGlyphShape(uint fontIndex, int glyph)
    {
        EnsureNotDisposed();
        var firstPassCode = new ErrorCode(FontNativeGenerated.IfontFileGetGlyphShape(_ifontFile, fontIndex, glyph, IntPtr.Zero, 0, out var count));
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
        using var pinnedResult = PinnedShortArray.Create(result);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.IfontFileGetGlyphShape(_ifontFile, fontIndex, glyph, pinnedResult.Pointer, count, out count)));
        return result;
    }

    public RectI GetGlyphBoundingBox(uint fontIndex, int glyph)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.IfontFileGetGlyphBoundingBox(_ifontFile, fontIndex, glyph, out var rect)));
        return rect;
    }

    public RectI GetGlyphBitmapBox(uint fontIndex, int glyph, float scaleX, float scaleY, float shiftX, float shiftY)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.IfontFileGetGlyphBitmapBox(_ifontFile, fontIndex, glyph, scaleX, scaleY, shiftX, shiftY, out var rect)));
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
        using var pinnedResult = PinnedByteArray.Create(result);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(FontNativeGenerated.IfontFileRenderGlyphBitmap(_ifontFile, fontIndex, glyph, pinnedResult.Pointer, width, height, rowPitch, scaleX, scaleY, shiftX, shiftY)));
        return result;
    }

    internal static IntPtr GetNativeFontPointer(IFontFile fontFile)
    {
        ArgumentNullException.ThrowIfNull(fontFile);
        if (fontFile is not NativeFontFile nativeFontFile)
        {
            throw new ArgumentException("The font file must be created by Luna.Font.", nameof(fontFile));
        }
        nativeFontFile.EnsureNotDisposed();
        return nativeFontFile._ifontFile;
    }

    private readonly struct PinnedByteArray : IDisposable
    {
        private readonly GCHandle m_handle;

        public IntPtr Pointer { get; }

        private PinnedByteArray(GCHandle handle, IntPtr pointer)
        {
            m_handle = handle;
            Pointer = pointer;
        }

        public static PinnedByteArray Create(byte[] data)
        {
            if (data.Length == 0)
            {
                return new PinnedByteArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedByteArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (m_handle.IsAllocated)
            {
                m_handle.Free();
            }
        }
    }

    private readonly struct PinnedShortArray : IDisposable
    {
        private readonly GCHandle m_handle;

        public IntPtr Pointer { get; }

        private PinnedShortArray(GCHandle handle, IntPtr pointer)
        {
            m_handle = handle;
            Pointer = pointer;
        }

        public static PinnedShortArray Create(short[] data)
        {
            if (data.Length == 0)
            {
                return new PinnedShortArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedShortArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (m_handle.IsAllocated)
            {
                m_handle.Free();
            }
        }
    }
}
