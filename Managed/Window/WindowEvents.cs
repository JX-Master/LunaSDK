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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.WindowEventGetWindow(nativeEvent.EventObject, out var nativeWindow)));
        return new NativeWindow(nativeWindow.Object, nativeWindow.IWindow, retain: true);
    }

    public static bool GetDoClose(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowRequestCloseEvent);
        return WindowNative.WindowRequestCloseEventGetDoClose(nativeEvent.EventObject) != 0;
    }

    public static void SetDoClose(IObject eventObject, bool doClose)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowRequestCloseEvent);
        WindowNative.WindowRequestCloseEventSetDoClose(nativeEvent.EventObject, doClose ? 1 : 0);
    }

    public static Size2U GetResizeSize(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowResizeEvent);
        WindowNative.WindowResizeEventGetSize(nativeEvent.EventObject, out var width, out var height);
        return new Size2U(width, height);
    }

    public static Size2U GetFramebufferResizeSize(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowFramebufferResizeEvent);
        WindowNative.WindowFramebufferResizeEventGetSize(nativeEvent.EventObject, out var width, out var height);
        return new Size2U(width, height);
    }

    public static (int X, int Y) GetMovePosition(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMoveEvent);
        WindowNative.WindowMoveEventGetPosition(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    public static uint GetKeyDownKey(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowKeyDownEvent);
        return WindowNative.WindowKeyDownEventGetKey(nativeEvent.EventObject);
    }

    public static uint GetKeyUpKey(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowKeyUpEvent);
        return WindowNative.WindowKeyUpEventGetKey(nativeEvent.EventObject);
    }

    public static string GetInputText(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowInputTextEvent);
        return Marshal.PtrToStringUTF8(WindowNative.WindowInputTextEventGetText(nativeEvent.EventObject)) ?? string.Empty;
    }

    public static (int X, int Y) GetMouseMovePosition(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMouseMoveEvent);
        WindowNative.WindowMouseMoveEventGetPosition(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    public static uint GetMouseDownButton(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMouseDownEvent);
        return WindowNative.WindowMouseDownEventGetButton(nativeEvent.EventObject);
    }

    public static uint GetMouseUpButton(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowMouseUpEvent);
        return WindowNative.WindowMouseUpEventGetButton(nativeEvent.EventObject);
    }

    public static (float X, float Y) GetScrollDelta(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowScrollEvent);
        WindowNative.WindowScrollEventGetDelta(nativeEvent.EventObject, out var x, out var y);
        return (x, y);
    }

    public static (ulong Id, float X, float Y) GetTouchDownPoint(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowTouchDownEvent);
        WindowNative.WindowTouchDownEventGetPoint(nativeEvent.EventObject, out var id, out var x, out var y);
        return (id, x, y);
    }

    public static (ulong Id, float X, float Y) GetTouchMovePoint(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowTouchMoveEvent);
        WindowNative.WindowTouchMoveEventGetPoint(nativeEvent.EventObject, out var id, out var x, out var y);
        return (id, x, y);
    }

    public static (ulong Id, float X, float Y) GetTouchUpPoint(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowTouchUpEvent);
        WindowNative.WindowTouchUpEventGetPoint(nativeEvent.EventObject, out var id, out var x, out var y);
        return (id, x, y);
    }

    public static ulong GetDropFileCount(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowDropFilesEvent);
        return WindowNative.WindowDropFilesEventGetFileCount(nativeEvent.EventObject);
    }

    public static string GetDropFile(IObject eventObject, ulong index)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowDropFilesEvent);
        return Marshal.PtrToStringUTF8(WindowNative.WindowDropFilesEventGetFile(nativeEvent.EventObject, index)) ?? string.Empty;
    }

    public static (float X, float Y) GetDropPosition(IObject eventObject)
    {
        var nativeEvent = RequireType(eventObject, WindowEventTypes.WindowDropFilesEvent);
        WindowNative.WindowDropFilesEventGetPosition(nativeEvent.EventObject, out var x, out var y);
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
