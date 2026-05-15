using System;
using System.Runtime.InteropServices;
using Luna.ImGui.Internal;
using Luna.Runtime.Internal;

namespace Luna.ImGui;

public sealed class InputTextCallbackData
{
    private readonly IntPtr _nativeData;

    internal InputTextCallbackData(IntPtr nativeData)
    {
        _nativeData = nativeData == IntPtr.Zero ? throw new ArgumentNullException(nameof(nativeData)) : nativeData;
    }

    public InputTextFlags EventFlag => (InputTextFlags)ImGuiNative.InputTextCallbackDataGetEventFlag(_nativeData);

    public InputTextFlags Flags => (InputTextFlags)ImGuiNative.InputTextCallbackDataGetFlags(_nativeData);

    public uint EventChar
    {
        get => ImGuiNative.InputTextCallbackDataGetEventChar(_nativeData);
        set => ImGuiNative.InputTextCallbackDataSetEventChar(_nativeData, value);
    }

    public uint EventKey => ImGuiNative.InputTextCallbackDataGetEventKey(_nativeData);

    public string Text
    {
        get
        {
            var value = ImGuiNative.InputTextCallbackDataGetText(_nativeData);
            try
            {
                return value == IntPtr.Zero ? string.Empty : Marshal.PtrToStringUTF8(value) ?? string.Empty;
            }
            finally
            {
                if (value != IntPtr.Zero)
                {
                    RuntimeNativeGenerated.FreeBuffer(value);
                }
            }
        }
        set
        {
            ImGuiNative.InputTextCallbackDataSetText(_nativeData, value ?? string.Empty);
        }
    }

    public int BufferSize => ImGuiNative.InputTextCallbackDataGetBufferSize(_nativeData);

    public int CursorPos
    {
        get => ImGuiNative.InputTextCallbackDataGetCursorPos(_nativeData);
        set => ImGuiNative.InputTextCallbackDataSetCursorPos(_nativeData, value);
    }

    public int SelectionStart
    {
        get => ImGuiNative.InputTextCallbackDataGetSelectionStart(_nativeData);
        set => ImGuiNative.InputTextCallbackDataSetSelectionStart(_nativeData, value);
    }

    public int SelectionEnd
    {
        get => ImGuiNative.InputTextCallbackDataGetSelectionEnd(_nativeData);
        set => ImGuiNative.InputTextCallbackDataSetSelectionEnd(_nativeData, value);
    }

    public bool HasSelection => ImGuiNative.InputTextCallbackDataHasSelection(_nativeData) != 0;

    public void DeleteChars(int position, int byteCount)
    {
        ImGuiNative.InputTextCallbackDataDeleteChars(_nativeData, position, byteCount);
    }

    public void InsertChars(int position, string text)
    {
        ImGuiNative.InputTextCallbackDataInsertChars(_nativeData, position, text ?? string.Empty);
    }

    public void SelectAll()
    {
        ImGuiNative.InputTextCallbackDataSelectAll(_nativeData);
    }

    public void ClearSelection()
    {
        ImGuiNative.InputTextCallbackDataClearSelection(_nativeData);
    }
}
