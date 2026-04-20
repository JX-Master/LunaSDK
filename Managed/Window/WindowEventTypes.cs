using Luna.Runtime;
using Luna.Window.Internal;
using RuntimeType = Luna.Runtime.Type;

namespace Luna.Window;

public static class WindowEventTypes
{
    public static RuntimeType WindowEvent => FromHandle(WindowNative.GetWindowEventType());

    public static RuntimeType WindowRequestCloseEvent => FromHandle(WindowNative.GetWindowRequestCloseEventType());

    public static RuntimeType WindowClosedEvent => FromHandle(WindowNative.GetWindowClosedEventType());

    public static RuntimeType WindowInputFocusEvent => FromHandle(WindowNative.GetWindowInputFocusEventType());

    public static RuntimeType WindowLoseInputFocusEvent => FromHandle(WindowNative.GetWindowLoseInputFocusEventType());

    public static RuntimeType WindowShowEvent => FromHandle(WindowNative.GetWindowShowEventType());

    public static RuntimeType WindowHideEvent => FromHandle(WindowNative.GetWindowHideEventType());

    public static RuntimeType WindowResizeEvent => FromHandle(WindowNative.GetWindowResizeEventType());

    public static RuntimeType WindowFramebufferResizeEvent => FromHandle(WindowNative.GetWindowFramebufferResizeEventType());

    public static RuntimeType WindowMoveEvent => FromHandle(WindowNative.GetWindowMoveEventType());

    public static RuntimeType WindowDpiScaleChangedEvent => FromHandle(WindowNative.GetWindowDpiScaleChangedEventType());

    public static RuntimeType WindowKeyDownEvent => FromHandle(WindowNative.GetWindowKeyDownEventType());

    public static RuntimeType WindowKeyUpEvent => FromHandle(WindowNative.GetWindowKeyUpEventType());

    public static RuntimeType WindowInputTextEvent => FromHandle(WindowNative.GetWindowInputTextEventType());

    public static RuntimeType WindowMouseEnterEvent => FromHandle(WindowNative.GetWindowMouseEnterEventType());

    public static RuntimeType WindowMouseLeaveEvent => FromHandle(WindowNative.GetWindowMouseLeaveEventType());

    public static RuntimeType WindowMouseMoveEvent => FromHandle(WindowNative.GetWindowMouseMoveEventType());

    public static RuntimeType WindowMouseDownEvent => FromHandle(WindowNative.GetWindowMouseDownEventType());

    public static RuntimeType WindowMouseUpEvent => FromHandle(WindowNative.GetWindowMouseUpEventType());

    public static RuntimeType WindowScrollEvent => FromHandle(WindowNative.GetWindowScrollEventType());

    public static RuntimeType WindowTouchDownEvent => FromHandle(WindowNative.GetWindowTouchDownEventType());

    public static RuntimeType WindowTouchMoveEvent => FromHandle(WindowNative.GetWindowTouchMoveEventType());

    public static RuntimeType WindowTouchUpEvent => FromHandle(WindowNative.GetWindowTouchUpEventType());

    public static RuntimeType WindowDropFilesEvent => FromHandle(WindowNative.GetWindowDropFilesEventType());

    public static RuntimeType ApplicationEvent => FromHandle(WindowNative.GetApplicationEventType());

    public static RuntimeType ApplicationDidEnterForegroundEvent => FromHandle(WindowNative.GetApplicationDidEnterForegroundEventType());

    public static RuntimeType ApplicationWillEnterForegroundEvent => FromHandle(WindowNative.GetApplicationWillEnterForegroundEventType());

    public static RuntimeType ApplicationDidEnterBackgroundEvent => FromHandle(WindowNative.GetApplicationDidEnterBackgroundEventType());

    public static RuntimeType ApplicationWillEnterBackgroundEvent => FromHandle(WindowNative.GetApplicationWillEnterBackgroundEventType());

    public static RuntimeType ApplicationWillTerminateEvent => FromHandle(WindowNative.GetApplicationWillTerminateEventType());

    public static RuntimeType ApplicationDidReceiveMemoryWarningEvent => FromHandle(WindowNative.GetApplicationDidReceiveMemoryWarningEventType());

    public static RuntimeType ScreenKeyboardShownEvent => FromHandle(WindowNative.GetScreenKeyboardShownEventType());

    public static RuntimeType ScreenKeyboardHiddenEvent => FromHandle(WindowNative.GetScreenKeyboardHiddenEventType());

    private static RuntimeType FromHandle(System.IntPtr handle)
    {
        return RuntimeTypes.FromNativeHandle(handle);
    }
}
