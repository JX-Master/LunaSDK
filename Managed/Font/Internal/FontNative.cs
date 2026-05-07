using System;
using System.Runtime.InteropServices;

namespace Luna.Font.Internal;

internal static class FontNative
{
    private const string LibraryName = "LunaFontC";

    [DllImport(LibraryName, EntryPoint = "luna_font_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_font_load_ttf_font_file")]
    internal static extern UIntPtr LoadTtfFontFile(
        [In] byte[] data,
        ulong dataSize,
        out NativeFontHandle outFontFile);

    [DllImport(LibraryName, EntryPoint = "luna_font_get_default_font")]
    internal static extern UIntPtr GetDefaultFont(out NativeFontHandle outFontFile);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_data")]
    internal static extern UIntPtr IFontFileGetData(
        IntPtr self,
        out IntPtr outData,
        out ulong outSize);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_num_fonts")]
    internal static extern uint IFontFileGetNumFonts(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_find_glyph")]
    internal static extern int IFontFileFindGlyph(IntPtr self, uint fontIndex, uint codepoint);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_scale_for_pixel_height")]
    internal static extern float IFontFileScaleForPixelHeight(IntPtr self, uint fontIndex, float pixels);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_vmetrics")]
    internal static extern void IFontFileGetVMetrics(IntPtr self, uint fontIndex, out VMetrics outMetrics);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_glyph_hmetrics")]
    internal static extern void IFontFileGetGlyphHMetrics(IntPtr self, uint fontIndex, int glyph, out GlyphHMetrics outMetrics);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_kern_advance")]
    internal static extern int IFontFileGetKernAdvance(IntPtr self, uint fontIndex, int glyph1, int glyph2);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_glyph_shape")]
    internal static extern UIntPtr IFontFileGetGlyphShape(
        IntPtr self,
        uint fontIndex,
        int glyph,
        [Out] short[] outCommands,
        ulong capacity,
        out ulong outCount);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_glyph_bounding_box")]
    internal static extern UIntPtr IFontFileGetGlyphBoundingBox(
        IntPtr self,
        uint fontIndex,
        int glyph,
        out RectI outRect);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_get_glyph_bitmap_box")]
    internal static extern UIntPtr IFontFileGetGlyphBitmapBox(
        IntPtr self,
        uint fontIndex,
        int glyph,
        float scaleX,
        float scaleY,
        float shiftX,
        float shiftY,
        out RectI outRect);

    [DllImport(LibraryName, EntryPoint = "luna_font_ifont_file_render_glyph_bitmap")]
    internal static extern UIntPtr IFontFileRenderGlyphBitmap(
        IntPtr self,
        uint fontIndex,
        int glyph,
        [Out] byte[] output,
        int outW,
        int outH,
        int outRowPitch,
        float scaleX,
        float scaleY,
        float shiftX,
        float shiftY);
}
