#if LUNA_PLATFORM_DESKTOP
using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Window.Internal;

namespace Luna.Window;

public static class WindowDisplays
{
    public static WindowDisplay PrimaryDisplay => new(WindowNative.DisplayGetPrimary());

    public static WindowDisplay[] GetDisplays()
    {
        WindowNative.DisplayGetAll(null, 0, out var count);
        if (count == 0)
        {
            return Array.Empty<WindowDisplay>();
        }

        var capacity = checked((int)count);
        var handles = new IntPtr[capacity];
        WindowNative.DisplayGetAll(handles, (ulong)handles.Length, out var writtenCount);
        var result = new WindowDisplay[Math.Min(handles.Length, checked((int)writtenCount))];
        for (var i = 0; i < result.Length; ++i)
        {
            result[i] = new WindowDisplay(handles[i]);
        }
        return result;
    }

    public static DisplayVideoMode[] GetSupportedVideoModes(WindowDisplay display)
    {
        EnsureValid(display);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.DisplayGetSupportedVideoModes(display.Handle, null, 0, out var count)));
        if (count == 0)
        {
            return Array.Empty<DisplayVideoMode>();
        }

        var result = new DisplayVideoMode[checked((int)count)];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.DisplayGetSupportedVideoModes(display.Handle, result, (ulong)result.Length, out var writtenCount)));
        if (writtenCount < (ulong)result.Length)
        {
            Array.Resize(ref result, checked((int)writtenCount));
        }
        return result;
    }

    public static DisplayVideoMode GetVideoMode(WindowDisplay display)
    {
        EnsureValid(display);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.DisplayGetVideoMode(display.Handle, out var mode)));
        return mode;
    }

    public static Point2I GetPosition(WindowDisplay display)
    {
        EnsureValid(display);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.DisplayGetPosition(display.Handle, out var position)));
        return position;
    }

    public static RectI GetWorkingArea(WindowDisplay display)
    {
        EnsureValid(display);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.DisplayGetWorkingArea(display.Handle, out var rect)));
        return rect;
    }

    public static string GetName(WindowDisplay display)
    {
        EnsureValid(display);
        var name = IntPtr.Zero;
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.DisplayGetName(display.Handle, out name)));
        try
        {
            return Marshal.PtrToStringUTF8(name) ?? string.Empty;
        }
        finally
        {
            WindowNative.FreeString(name);
        }
    }

    private static void EnsureValid(WindowDisplay display)
    {
        if (!display.IsValid)
        {
            throw new ArgumentException("Display handle is invalid.", nameof(display));
        }
    }
}
#endif
