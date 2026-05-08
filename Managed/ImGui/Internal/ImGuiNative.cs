using System;
using System.Runtime.InteropServices;
using Luna.RHI.Internal;

namespace Luna.ImGui.Internal;

internal static class ImGuiNative
{
    private const string LibraryName = "LunaImGuiC";

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate int InputTextCallback(IntPtr data, IntPtr userdata);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_imgui_set_active_window")]
    internal static extern void SetActiveWindow(IntPtr windowObject);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_handle_window_event")]
    internal static extern int HandleWindowEvent(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_update_io")]
    internal static extern void UpdateIo();

    [DllImport(LibraryName, EntryPoint = "luna_imgui_add_default_font")]
    internal static extern void AddDefaultFont(float fontSize);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_get_glyph_ranges_default")]
    internal static extern void GetGlyphRangesDefault([Out] NativeGlyphRange[]? ranges, ulong capacity, out ulong count);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_new_sampled_image")]
    internal static extern UIntPtr NewSampledImage(IntPtr textureObject, in NativeSamplerDesc samplerDesc, out NativeSampledImageHandle outImage);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_sampled_image_get_texture")]
    internal static extern UIntPtr SampledImageGetTexture(IntPtr sampledImage, out NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_sampled_image_set_texture")]
    internal static extern void SampledImageSetTexture(IntPtr sampledImage, IntPtr textureObject);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_sampled_image_get_sampler")]
    internal static extern UIntPtr SampledImageGetSampler(IntPtr sampledImage, out NativeSamplerDesc outSampler);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_sampled_image_set_sampler")]
    internal static extern void SampledImageSetSampler(IntPtr sampledImage, in NativeSamplerDesc samplerDesc);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_new_frame")]
    internal static extern void NewFrame();

    [DllImport(LibraryName, EntryPoint = "luna_imgui_show_demo_window")]
    internal static extern void ShowDemoWindow();

    [DllImport(LibraryName, EntryPoint = "luna_imgui_begin")]
    internal static extern int Begin([MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_end")]
    internal static extern void End();

    [DllImport(LibraryName, EntryPoint = "luna_imgui_text")]
    internal static extern void Text([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_image_texture")]
    internal static extern void ImageTexture(IntPtr textureObject, in NativeFloat2 imageSize, in NativeFloat2 uv0, in NativeFloat2 uv1);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_image_sampled_image")]
    internal static extern void ImageSampledImage(IntPtr sampledImage, in NativeFloat2 imageSize, in NativeFloat2 uv0, in NativeFloat2 uv1);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_image_button_texture")]
    internal static extern int ImageButtonTexture([MarshalAs(UnmanagedType.LPUTF8Str)] string id, IntPtr textureObject, in NativeFloat2 imageSize, in NativeFloat2 uv0, in NativeFloat2 uv1, in NativeFloat4 bgColor, in NativeFloat4 tintColor);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_image_button_sampled_image")]
    internal static extern int ImageButtonSampledImage([MarshalAs(UnmanagedType.LPUTF8Str)] string id, IntPtr sampledImage, in NativeFloat2 imageSize, in NativeFloat2 uv0, in NativeFloat2 uv1, in NativeFloat4 bgColor, in NativeFloat4 tintColor);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text")]
    internal static extern UIntPtr InputText(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string value,
        uint flags,
        InputTextCallback? callback,
        IntPtr userdata,
        out int changed,
        out IntPtr outValue);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_multiline")]
    internal static extern UIntPtr InputTextMultiline(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string value,
        in NativeFloat2 size,
        uint flags,
        InputTextCallback? callback,
        IntPtr userdata,
        out int changed,
        out IntPtr outValue);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_with_hint")]
    internal static extern UIntPtr InputTextWithHint(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string label,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string hint,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string value,
        uint flags,
        InputTextCallback? callback,
        IntPtr userdata,
        out int changed,
        out IntPtr outValue);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_event_flag")]
    internal static extern uint InputTextCallbackDataGetEventFlag(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_flags")]
    internal static extern uint InputTextCallbackDataGetFlags(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_event_char")]
    internal static extern uint InputTextCallbackDataGetEventChar(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_set_event_char")]
    internal static extern void InputTextCallbackDataSetEventChar(IntPtr data, uint ch);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_event_key")]
    internal static extern uint InputTextCallbackDataGetEventKey(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_text")]
    internal static extern IntPtr InputTextCallbackDataGetText(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_set_text")]
    internal static extern void InputTextCallbackDataSetText(IntPtr data, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_buffer_size")]
    internal static extern int InputTextCallbackDataGetBufferSize(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_cursor_pos")]
    internal static extern int InputTextCallbackDataGetCursorPos(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_set_cursor_pos")]
    internal static extern void InputTextCallbackDataSetCursorPos(IntPtr data, int value);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_selection_start")]
    internal static extern int InputTextCallbackDataGetSelectionStart(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_set_selection_start")]
    internal static extern void InputTextCallbackDataSetSelectionStart(IntPtr data, int value);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_get_selection_end")]
    internal static extern int InputTextCallbackDataGetSelectionEnd(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_set_selection_end")]
    internal static extern void InputTextCallbackDataSetSelectionEnd(IntPtr data, int value);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_has_selection")]
    internal static extern int InputTextCallbackDataHasSelection(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_delete_chars")]
    internal static extern void InputTextCallbackDataDeleteChars(IntPtr data, int position, int byteCount);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_insert_chars")]
    internal static extern void InputTextCallbackDataInsertChars(IntPtr data, int position, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_select_all")]
    internal static extern void InputTextCallbackDataSelectAll(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_input_text_callback_data_clear_selection")]
    internal static extern void InputTextCallbackDataClearSelection(IntPtr data);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_gizmo")]
    internal static extern void Gizmo(
        ref NativeMatrix4x4 worldMatrix,
        in NativeMatrix4x4 view,
        in NativeMatrix4x4 projection,
        in RectF viewportRect,
        uint operation,
        uint mode,
        float snap,
        int enabled,
        int orthographic,
        out NativeMatrix4x4 deltaMatrix,
        out int isMouseHover,
        out int isMouseMoving);

    [DllImport(LibraryName, EntryPoint = "luna_imgui_render")]
    internal static extern void Render();

    [DllImport(LibraryName, EntryPoint = "luna_imgui_render_draw_data")]
    internal static extern UIntPtr RenderDrawData(IntPtr commandBufferObject, IntPtr renderTargetObject);
}
