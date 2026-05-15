using Luna.Runtime;
using Luna.Window.Internal;
using RuntimeType = Luna.Runtime.Type;

namespace Luna.Window;

public static class WindowEventTypes
{
    public static RuntimeType WindowEvent => FromHandle(WindowNativeGenerated.GetWindowEventType());

    public static RuntimeType WindowRequestCloseEvent => FromHandle(WindowNativeGenerated.GetWindowRequestCloseEventType());

    public static RuntimeType WindowClosedEvent => FromHandle(WindowNativeGenerated.GetWindowClosedEventType());

    public static RuntimeType WindowInputFocusEvent => FromHandle(WindowNativeGenerated.GetWindowInputFocusEventType());

    public static RuntimeType WindowLoseInputFocusEvent => FromHandle(WindowNativeGenerated.GetWindowLoseInputFocusEventType());

    public static RuntimeType WindowShowEvent => FromHandle(WindowNativeGenerated.GetWindowShowEventType());

    public static RuntimeType WindowHideEvent => FromHandle(WindowNativeGenerated.GetWindowHideEventType());

    public static RuntimeType WindowResizeEvent => FromHandle(WindowNativeGenerated.GetWindowResizeEventType());

    public static RuntimeType WindowFramebufferResizeEvent => FromHandle(WindowNativeGenerated.GetWindowFramebufferResizeEventType());

    public static RuntimeType WindowMoveEvent => FromHandle(WindowNativeGenerated.GetWindowMoveEventType());

    public static RuntimeType WindowDpiScaleChangedEvent => FromHandle(WindowNativeGenerated.GetWindowDpiScaleChangedEventType());

    public static RuntimeType WindowKeyDownEvent => FromHandle(WindowNativeGenerated.GetWindowKeyDownEventType());

    public static RuntimeType WindowKeyUpEvent => FromHandle(WindowNativeGenerated.GetWindowKeyUpEventType());

    public static RuntimeType WindowInputTextEvent => FromHandle(WindowNativeGenerated.GetWindowInputTextEventType());

    public static RuntimeType WindowMouseEnterEvent => FromHandle(WindowNativeGenerated.GetWindowMouseEnterEventType());

    public static RuntimeType WindowMouseLeaveEvent => FromHandle(WindowNativeGenerated.GetWindowMouseLeaveEventType());

    public static RuntimeType WindowMouseMoveEvent => FromHandle(WindowNativeGenerated.GetWindowMouseMoveEventType());

    public static RuntimeType WindowMouseDownEvent => FromHandle(WindowNativeGenerated.GetWindowMouseDownEventType());

    public static RuntimeType WindowMouseUpEvent => FromHandle(WindowNativeGenerated.GetWindowMouseUpEventType());

    public static RuntimeType WindowScrollEvent => FromHandle(WindowNativeGenerated.GetWindowScrollEventType());

    public static RuntimeType WindowTouchDownEvent => FromHandle(WindowNativeGenerated.GetWindowTouchDownEventType());

    public static RuntimeType WindowTouchMoveEvent => FromHandle(WindowNativeGenerated.GetWindowTouchMoveEventType());

    public static RuntimeType WindowTouchUpEvent => FromHandle(WindowNativeGenerated.GetWindowTouchUpEventType());

    public static RuntimeType WindowDropFilesEvent => FromHandle(WindowNativeGenerated.GetWindowDropFilesEventType());

    public static RuntimeType ApplicationEvent => FromHandle(WindowNativeGenerated.GetApplicationEventType());

    public static RuntimeType ApplicationDidEnterForegroundEvent => FromHandle(WindowNativeGenerated.GetApplicationDidEnterForegroundEventType());

    public static RuntimeType ApplicationWillEnterForegroundEvent => FromHandle(WindowNativeGenerated.GetApplicationWillEnterForegroundEventType());

    public static RuntimeType ApplicationDidEnterBackgroundEvent => FromHandle(WindowNativeGenerated.GetApplicationDidEnterBackgroundEventType());

    public static RuntimeType ApplicationWillEnterBackgroundEvent => FromHandle(WindowNativeGenerated.GetApplicationWillEnterBackgroundEventType());

    public static RuntimeType ApplicationWillTerminateEvent => FromHandle(WindowNativeGenerated.GetApplicationWillTerminateEventType());

    public static RuntimeType ApplicationDidReceiveMemoryWarningEvent => FromHandle(WindowNativeGenerated.GetApplicationDidReceiveMemoryWarningEventType());

    public static RuntimeType ScreenKeyboardShownEvent => FromHandle(WindowNativeGenerated.GetScreenKeyboardShownEventType());

    public static RuntimeType ScreenKeyboardHiddenEvent => FromHandle(WindowNativeGenerated.GetScreenKeyboardHiddenEventType());

    private static RuntimeType FromHandle(System.IntPtr handle)
    {
        return RuntimeTypes.FromNativeHandle(handle);
    }
}
