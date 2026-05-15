using System;
using System.Runtime.ExceptionServices;
using Luna.Runtime;
using Luna.Window.Internal;

namespace Luna.Window;

public static class WindowModule
{
    public const int DefaultPosition = int.MaxValue;
    private static readonly WindowEventHandler NativeEventHandler = OnNativeEvent;
    private static Action<IObject>? s_eventHandler;
    private static ExceptionDispatchInfo? s_pendingEventException;

    public static void Init(string appName = "Luna")
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the Window module.");
        }

        ArgumentNullException.ThrowIfNull(appName);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.InitModule(appName)));
    }

    public static IWindow CreateWindow(WindowCreationDesc desc)
    {
        ArgumentNullException.ThrowIfNull(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.New(
            desc.Title,
            desc.X,
            desc.Y,
            desc.Width,
            desc.Height,
            (uint)desc.StyleFlags,
            (uint)desc.CreationFlags,
            out var nativeWindow)));

        return new NativeWindow(nativeWindow.Object, nativeWindow.IWindow, retain: false);
    }

    internal static IWindow WrapNativeWindow(IntPtr nativeObject, IntPtr nativeWindow, bool retain)
    {
        return new NativeWindow(nativeObject, nativeWindow, retain);
    }

    public static void PollEvents(bool waitEvents = false)
    {
        WindowNativeGenerated.PollEvents(waitEvents ? 1 : 0);
        ThrowPendingEventException();
    }

    public static void SetEventHandler(Action<IObject>? eventHandler)
    {
        s_eventHandler = eventHandler;
        WindowNativeGenerated.SetEventHandler(
            eventHandler is null ? IntPtr.Zero : System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(NativeEventHandler),
            IntPtr.Zero);
    }

    private static void OnNativeEvent(IntPtr eventObject, IntPtr userdata)
    {
        try
        {
            if (s_eventHandler is null)
            {
                return;
            }
            using var managedEvent = new NativeWindowEvent(eventObject, retain: true);
            s_eventHandler(managedEvent);
        }
        catch (Exception ex)
        {
            s_pendingEventException ??= ExceptionDispatchInfo.Capture(ex);
        }
    }

    private static void ThrowPendingEventException()
    {
        var exception = s_pendingEventException;
        if (exception is null)
        {
            return;
        }
        s_pendingEventException = null;
        exception.Throw();
    }
}
