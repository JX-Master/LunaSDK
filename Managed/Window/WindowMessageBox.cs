using System;
using Luna.Runtime;
using Luna.Window.Internal;

namespace Luna.Window;

public static class WindowMessageBox
{
    public static MessageBoxButton Show(
        string text,
        string caption,
        MessageBoxType type = MessageBoxType.Ok,
        MessageBoxIcon icon = MessageBoxIcon.None)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(caption);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.MessageBox(
            text,
            caption,
            (uint)type,
            (uint)icon,
            out var button)));
        return (MessageBoxButton)button;
    }
}
