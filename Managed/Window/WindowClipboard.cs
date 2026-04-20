using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Window.Internal;

namespace Luna.Window;

public static class WindowClipboard
{
    public static string Text
    {
        get
        {
            var text = IntPtr.Zero;
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.ClipboardGetText(out text)));
            try
            {
                return Marshal.PtrToStringUTF8(text) ?? string.Empty;
            }
            finally
            {
                WindowNative.FreeString(text);
            }
        }
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.ClipboardSetText(value)));
        }
    }
}
