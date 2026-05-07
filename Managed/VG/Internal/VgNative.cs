using System;
using System.Runtime.InteropServices;

namespace Luna.VG.Internal;

internal static class VgNative
{
    private const string LibraryName = "LunaVGC";

    [DllImport(LibraryName, EntryPoint = "luna_vg_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_vg_arrange_text")]
    internal static extern UIntPtr ArrangeText(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string text,
        ulong textLen,
        [In] NativeTextArrangeSection[] sections,
        ulong numSections,
        in RectF boundingRect,
        byte verticalAlignment,
        byte horizontalAlignment,
        out IntPtr outResult);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_free")]
    internal static extern void TextArrangeResultFree(IntPtr result);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_get_bounding_rect")]
    internal static extern UIntPtr TextArrangeResultGetBoundingRect(IntPtr result, out RectF outBoundingRect);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_get_overflow")]
    internal static extern int TextArrangeResultGetOverflow(IntPtr result);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_get_num_lines")]
    internal static extern ulong TextArrangeResultGetNumLines(IntPtr result);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_get_line")]
    internal static extern UIntPtr TextArrangeResultGetLine(IntPtr result, ulong index, out NativeTextLineArrangeResult outLine);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_get_num_glyphs")]
    internal static extern ulong TextArrangeResultGetNumGlyphs(IntPtr result, ulong lineIndex);

    [DllImport(LibraryName, EntryPoint = "luna_vg_text_arrange_result_get_glyph")]
    internal static extern UIntPtr TextArrangeResultGetGlyph(IntPtr result, ulong lineIndex, ulong glyphIndex, out NativeTextGlyphArrangeResult outGlyph);

    [DllImport(LibraryName, EntryPoint = "luna_vg_generate_text_arrange_result_draw_vertices")]
    internal static extern UIntPtr GenerateTextArrangeResultDrawVertices(
        IntPtr result,
        [In] NativeTextArrangeSection[] sections,
        ulong numSections,
        IntPtr fontAtlas,
        [Out] NativeVertex[]? outVertices,
        ulong vertexCapacity,
        out ulong outVertexCount,
        [Out] uint[]? outIndices,
        ulong indexCapacity,
        out ulong outIndexCount);

    [DllImport(LibraryName, EntryPoint = "luna_vg_commit_text_arrange_result")]
    internal static extern UIntPtr CommitTextArrangeResult(
        IntPtr result,
        [In] NativeTextArrangeSection[] sections,
        ulong numSections,
        IntPtr fontAtlas,
        IntPtr drawList);

    [DllImport(LibraryName, EntryPoint = "luna_vg_new_shape_buffer")]
    internal static extern UIntPtr NewShapeBuffer(out NativeShapeBufferHandle outShapeBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_buffer_get_points")]
    internal static extern UIntPtr ShapeBufferGetPoints(IntPtr self, [Out] float[]? outPoints, ulong capacity, out ulong outCount);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_buffer_set_points")]
    internal static extern UIntPtr ShapeBufferSetPoints(IntPtr self, [In] float[] points, ulong count);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_buffer_build")]
    internal static extern UIntPtr ShapeBufferBuild(IntPtr self, IntPtr device, out RHI.Internal.NativeRhiBufferHandle outBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_new_font_atlas")]
    internal static extern UIntPtr NewFontAtlas(out NativeFontAtlasHandle outFontAtlas);

    [DllImport(LibraryName, EntryPoint = "luna_vg_font_atlas_clear")]
    internal static extern void FontAtlasClear(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_font_atlas_get_font")]
    internal static extern UIntPtr FontAtlasGetFont(IntPtr self, out Font.Internal.NativeFontHandle outFont, out uint outIndex);

    [DllImport(LibraryName, EntryPoint = "luna_vg_font_atlas_set_font")]
    internal static extern UIntPtr FontAtlasSetFont(IntPtr self, IntPtr fontFile, uint index);

    [DllImport(LibraryName, EntryPoint = "luna_vg_font_atlas_get_shape_buffer")]
    internal static extern UIntPtr FontAtlasGetShapeBuffer(IntPtr self, out NativeShapeBufferHandle outShapeBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_font_atlas_get_glyph")]
    internal static extern UIntPtr FontAtlasGetGlyph(
        IntPtr self,
        uint codepoint,
        out ulong outFirstShapePoint,
        out ulong outNumShapePoints,
        out RectF outBoundingRect);

    [DllImport(LibraryName, EntryPoint = "luna_vg_get_font_glyph_shape")]
    internal static extern UIntPtr GetFontGlyphShape(
        IntPtr fontFile,
        uint fontIndex,
        uint codepoint,
        [Out] float[]? outShapePoints,
        ulong capacity,
        out ulong outCount,
        out RectF outBoundingRect);

    [DllImport(LibraryName, EntryPoint = "luna_vg_new_shape_draw_list")]
    internal static extern UIntPtr NewShapeDrawList(IntPtr device, out NativeShapeDrawListHandle outDrawList);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_device")]
    internal static extern UIntPtr ShapeDrawListGetDevice(IntPtr self, out RHI.Internal.NativeRhiDeviceHandle outDevice);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_reset")]
    internal static extern void ShapeDrawListReset(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_set_shape_buffer")]
    internal static extern void ShapeDrawListSetShapeBuffer(IntPtr self, IntPtr shapeBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_shape_buffer")]
    internal static extern UIntPtr ShapeDrawListGetShapeBuffer(IntPtr self, out NativeShapeBufferHandle outShapeBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_set_texture")]
    internal static extern void ShapeDrawListSetTexture(IntPtr self, IntPtr texture);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_texture")]
    internal static extern UIntPtr ShapeDrawListGetTexture(IntPtr self, out RHI.Internal.NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_set_sampler")]
    internal static extern void ShapeDrawListSetSampler(IntPtr self, in NativeSamplerDesc desc);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_reset_sampler")]
    internal static extern void ShapeDrawListResetSampler(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_sampler")]
    internal static extern UIntPtr ShapeDrawListGetSampler(IntPtr self, out NativeSamplerDesc outSampler);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_set_transform")]
    internal static extern UIntPtr ShapeDrawListSetTransform(IntPtr self, in NativeMatrix4x4 transform);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_transform")]
    internal static extern UIntPtr ShapeDrawListGetTransform(IntPtr self, out NativeMatrix4x4 outTransform);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_set_clip_rect")]
    internal static extern UIntPtr ShapeDrawListSetClipRect(IntPtr self, in RectF clipRect);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_clip_rect")]
    internal static extern UIntPtr ShapeDrawListGetClipRect(IntPtr self, out RectF outClipRect);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_draw_shape_raw")]
    internal static extern UIntPtr ShapeDrawListDrawShapeRaw(
        IntPtr self,
        [In] NativeVertex[] vertices,
        ulong numVertices,
        [In] uint[] indices,
        ulong numIndices);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_draw_shape")]
    internal static extern UIntPtr ShapeDrawListDrawShape(
        IntPtr self,
        uint beginCommand,
        uint numCommands,
        in NativeFloat2 minPosition,
        in NativeFloat2 maxPosition,
        in NativeFloat2 minShapeCoord,
        in NativeFloat2 maxShapeCoord,
        in NativeFloat4 color,
        in NativeFloat2 minTexCoord,
        in NativeFloat2 maxTexCoord);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_compile")]
    internal static extern UIntPtr ShapeDrawListCompile(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_vertex_buffer")]
    internal static extern UIntPtr ShapeDrawListGetVertexBuffer(IntPtr self, out RHI.Internal.NativeRhiBufferHandle outBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_vertex_buffer_size")]
    internal static extern uint ShapeDrawListGetVertexBufferSize(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_index_buffer")]
    internal static extern UIntPtr ShapeDrawListGetIndexBuffer(IntPtr self, out RHI.Internal.NativeRhiBufferHandle outBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_index_buffer_size")]
    internal static extern uint ShapeDrawListGetIndexBufferSize(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_draw_call_count")]
    internal static extern ulong ShapeDrawListGetDrawCallCount(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_draw_list_get_draw_call")]
    internal static extern UIntPtr ShapeDrawListGetDrawCall(IntPtr self, ulong index, out NativeShapeDrawCall outDrawCall);

    [DllImport(LibraryName, EntryPoint = "luna_vg_new_fill_shape_renderer")]
    internal static extern UIntPtr NewFillShapeRenderer(out NativeShapeRendererHandle outRenderer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_renderer_begin")]
    internal static extern UIntPtr ShapeRendererBegin(IntPtr self, IntPtr renderTarget);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_renderer_draw")]
    internal static extern UIntPtr ShapeRendererDraw(
        IntPtr self,
        IntPtr vertexBuffer,
        IntPtr indexBuffer,
        [In] NativeShapeDrawCall[] drawCalls,
        ulong numDrawCalls);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_renderer_draw_with_transform")]
    internal static extern UIntPtr ShapeRendererDrawWithTransform(
        IntPtr self,
        IntPtr vertexBuffer,
        IntPtr indexBuffer,
        [In] NativeShapeDrawCall[] drawCalls,
        ulong numDrawCalls,
        in NativeMatrix4x4 transformMatrix);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_renderer_end")]
    internal static extern UIntPtr ShapeRendererEnd(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_renderer_submit")]
    internal static extern UIntPtr ShapeRendererSubmit(IntPtr self, IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_rectangle_filled")]
    internal static extern UIntPtr ShapeBuilderAddRectangleFilled(IntPtr shapeBuffer, float minX, float minY, float maxX, float maxY);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_rectangle_bordered")]
    internal static extern UIntPtr ShapeBuilderAddRectangleBordered(IntPtr shapeBuffer, float minX, float minY, float maxX, float maxY, float borderWidth, float borderOffset);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_rounded_rectangle_filled")]
    internal static extern UIntPtr ShapeBuilderAddRoundedRectangleFilled(IntPtr shapeBuffer, float minX, float minY, float maxX, float maxY, float radius);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_rounded_rectangle_bordered")]
    internal static extern UIntPtr ShapeBuilderAddRoundedRectangleBordered(IntPtr shapeBuffer, float minX, float minY, float maxX, float maxY, float radius, float borderWidth, float borderOffset);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_triangle_filled")]
    internal static extern UIntPtr ShapeBuilderAddTriangleFilled(IntPtr shapeBuffer, float x1, float y1, float x2, float y2, float x3, float y3);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_triangle_bordered")]
    internal static extern UIntPtr ShapeBuilderAddTriangleBordered(IntPtr shapeBuffer, float x1, float y1, float x2, float y2, float x3, float y3, float borderWidth, float borderOffset);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_circle_filled")]
    internal static extern UIntPtr ShapeBuilderAddCircleFilled(IntPtr shapeBuffer, float centerX, float centerY, float radius);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_circle_bordered")]
    internal static extern UIntPtr ShapeBuilderAddCircleBordered(IntPtr shapeBuffer, float centerX, float centerY, float radius, float borderWidth, float borderOffset);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_axis_aligned_ellipse_filled")]
    internal static extern UIntPtr ShapeBuilderAddAxisAlignedEllipseFilled(IntPtr shapeBuffer, float centerX, float centerY, float radiusX, float radiusY);

    [DllImport(LibraryName, EntryPoint = "luna_vg_shape_builder_add_axis_aligned_ellipse_bordered")]
    internal static extern UIntPtr ShapeBuilderAddAxisAlignedEllipseBordered(IntPtr shapeBuffer, float centerX, float centerY, float radiusX, float radiusY, float borderWidth, float borderOffset);

    [DllImport(LibraryName, EntryPoint = "luna_vg_get_rect_shape_draw_vertices")]
    internal static extern UIntPtr GetRectShapeDrawVertices(
        [Out] NativeVertex[] outVertices,
        [Out] uint[] outIndices,
        uint beginCommand,
        uint numCommands,
        in NativeFloat2 minPosition,
        in NativeFloat2 maxPosition,
        in NativeFloat2 minShapeCoord,
        in NativeFloat2 maxShapeCoord,
        in NativeFloat4 color,
        in NativeFloat2 minTexCoord,
        in NativeFloat2 maxTexCoord);
}
