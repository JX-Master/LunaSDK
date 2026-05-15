using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Window.Internal;
using RuntimeType = Luna.Runtime.Type;

namespace Luna.Window;

public static class WindowEvents
{
    public static IWindow GetWindow(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowEvent);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.EventGetWindow(nativeEvent.EventObject, out var nativeWindow)));
        return new NativeWindow(nativeWindow.Object, nativeWindow.IWindow, retain: true);
    }

    public static bool GetDoClose(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowRequestCloseEvent);
        return WindowNativeGenerated.RequestCloseEventGetDoClose(nativeEvent.EventObject) != 0;
    }

    public static void SetDoClose(IObject eventObject, bool doClose)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowRequestCloseEvent);
        WindowNativeGenerated.RequestCloseEventSetDoClose(nativeEvent.EventObject, doClose ? 1 : 0);
    }

    public static Size2U GetResizeSize(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowResizeEvent);
        WindowNativeGenerated.ResizeEventGetSize(nativeEvent.EventObject, out var width, out var height);
        return new Size2U(width, height);
    }

    public static Size2U GetFramebufferResizeSize(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowFramebufferResizeEvent);
        WindowNativeGenerated.FramebufferResizeEventGetSize(nativeEvent.EventObject, out var width, out var height);
        return new Size2U(width, height);
    }

    public static (int X, int Y) GetMovePosition(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMoveEvent);
        WindowNativeGenerated.MoveEventGetPosition(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    public static uint GetKeyDownKey(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowKeyDownEvent);
        return WindowNativeGenerated.KeyDownEventGetKey(nativeEvent.EventObject);
    }

    public static uint GetKeyUpKey(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowKeyUpEvent);
        return WindowNativeGenerated.KeyUpEventGetKey(nativeEvent.EventObject);
    }

    public static string GetInputText(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowInputTextEvent);
        return Marshal.PtrToStringUTF8(WindowNativeGenerated.InputTextEventGetText(nativeEvent.EventObject)) ?? string.Empty;
    }

    public static (int X, int Y) GetMouseMovePosition(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMouseMoveEvent);
        WindowNativeGenerated.MouseMoveEventGetPosition(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    public static uint GetMouseDownButton(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMouseDownEvent);
        return WindowNativeGenerated.MouseDownEventGetButton(nativeEvent.EventObject);
    }

    public static uint GetMouseUpButton(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMouseUpEvent);
        return WindowNativeGenerated.MouseUpEventGetButton(nativeEvent.EventObject);
    }

    public static (float X, float Y) GetScrollDelta(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowScrollEvent);
        WindowNativeGenerated.ScrollEventGetDelta(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    public static (ulong Id, float X, float Y) GetTouchDownPoint(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowTouchDownEvent);
        WindowNativeGenerated.TouchDownEventGetPoint(nativeEvent.EventObject, out var id, out var x, out var y);
        return (id, x, y);
    }

    public static (ulong Id, float X, float Y) GetTouchMovePoint(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowTouchMoveEvent);
        WindowNativeGenerated.TouchMoveEventGetPoint(nativeEvent.EventObject, out var id, out var x, out var y);
        return (id, x, y);
    }

    public static (ulong Id, float X, float Y) GetTouchUpPoint(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowTouchUpEvent);
        WindowNativeGenerated.TouchUpEventGetPoint(nativeEvent.EventObject, out var id, out var x, out var y);
        return (id, x, y);
    }

    public static ulong GetDropFileCount(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowDropFilesEvent);
        return WindowNativeGenerated.DropFilesEventGetFileCount(nativeEvent.EventObject);
    }

    public static string GetDropFile(IObject eventObject, ulong index)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowDropFilesEvent);
        return Marshal.PtrToStringUTF8(WindowNativeGenerated.DropFilesEventGetFile(nativeEvent.EventObject, index)) ?? string.Empty;
    }

    public static (float X, float Y) GetDropPosition(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowDropFilesEvent);
        WindowNativeGenerated.DropFilesEventGetPosition(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    internal static NativeWindowEvent RequireNativeEvent(IObject eventObject)
    {
        return eventObject as NativeWindowEvent
            ?? throw new ArgumentException("The object was not created by the Luna.Window event binding.", nameof(eventObject));
    }

    private static NativeWindowEvent RequireType(IObject eventObject, RuntimeType type)
    {
        var nativeEvent = RequireNativeEvent(eventObject);
        if (!nativeEvent.IsA(type))
        {
            throw new InvalidOperationException($"Expected event type '{type.Name}', got '{nativeEvent.Type.Name}'.");
        }
        return nativeEvent;
    }
}
