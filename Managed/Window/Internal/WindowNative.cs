using System;
using System.Runtime.InteropServices;

namespace Luna.Window.Internal;

internal static class WindowNative
{
    private const string LibraryName = "LunaWindowC";

    internal delegate void EventHandler(IntPtr eventObject, IntPtr userdata);

    [StructLayout(LayoutKind.Sequential)]
    internal readonly struct FileDialogFilter
    {
        public FileDialogFilter(IntPtr name, IntPtr extensions, ulong extensionCount)
        {
            Name = name;
            Extensions = extensions;
            ExtensionCount = extensionCount;
        }

        public readonly IntPtr Name;
        public readonly IntPtr Extensions;
        public readonly ulong ExtensionCount;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal readonly struct StringList
    {
        public readonly IntPtr Items;
        public readonly ulong Count;
    }

    [DllImport(LibraryName, EntryPoint = "luna_window_init_module")]
    internal static extern UIntPtr InitModule(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string appName);

    [DllImport(LibraryName, EntryPoint = "luna_window_new")]
    internal static extern UIntPtr NewWindow(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string title,
        int x,
        int y,
        uint width,
        uint height,
        uint styleFlags,
        uint creationFlags,
        out NativeWindowHandle outWindow);

    [DllImport(LibraryName, EntryPoint = "luna_window_poll_events")]
    internal static extern void PollEvents(int waitEvents);

    [DllImport(LibraryName, EntryPoint = "luna_window_set_event_handler")]
    internal static extern void SetEventHandler(EventHandler? eventHandler, IntPtr userdata);

#if LUNA_PLATFORM_DESKTOP
    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_primary")]
    internal static extern IntPtr DisplayGetPrimary();

    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_all")]
    internal static extern void DisplayGetAll(
        [Out] IntPtr[]? outDisplays,
        ulong capacity,
        out ulong count);

    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_supported_video_modes")]
    internal static extern UIntPtr DisplayGetSupportedVideoModes(
        IntPtr display,
        [Out] DisplayVideoMode[]? outModes,
        ulong capacity,
        out ulong count);

    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_video_mode")]
    internal static extern UIntPtr DisplayGetVideoMode(IntPtr display, out DisplayVideoMode mode);

    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_position")]
    internal static extern UIntPtr DisplayGetPosition(IntPtr display, out Point2I position);

    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_working_area")]
    internal static extern UIntPtr DisplayGetWorkingArea(IntPtr display, out RectI rect);

    [DllImport(LibraryName, EntryPoint = "luna_window_display_get_name")]
    internal static extern UIntPtr DisplayGetName(IntPtr display, out IntPtr name);
#endif

    [DllImport(LibraryName, EntryPoint = "luna_window_clipboard_get_text")]
    internal static extern UIntPtr ClipboardGetText(out IntPtr text);

    [DllImport(LibraryName, EntryPoint = "luna_window_clipboard_set_text")]
    internal static extern UIntPtr ClipboardSetText([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

    [DllImport(LibraryName, EntryPoint = "luna_window_free_string")]
    internal static extern void FreeString(IntPtr text);

    [DllImport(LibraryName, EntryPoint = "luna_window_free_string_list")]
    internal static extern void FreeStringList(IntPtr texts, ulong count);

    [DllImport(LibraryName, EntryPoint = "luna_window_message_box")]
    internal static extern UIntPtr MessageBox(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string text,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string caption,
        uint type,
        uint icon,
        out uint button);

#if LUNA_PLATFORM_WINDOWS || LUNA_PLATFORM_MACOS
    [DllImport(LibraryName, EntryPoint = "luna_window_open_file_dialog")]
    internal static extern UIntPtr OpenFileDialog(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? title,
        [In] FileDialogFilter[]? filters,
        ulong filterCount,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? initialDirectory,
        uint flags,
        out StringList paths);

    [DllImport(LibraryName, EntryPoint = "luna_window_save_file_dialog")]
    internal static extern UIntPtr SaveFileDialog(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? title,
        [In] FileDialogFilter[]? filters,
        ulong filterCount,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? initialFilePath,
        uint flags,
        out IntPtr path);

    [DllImport(LibraryName, EntryPoint = "luna_window_open_dir_dialog")]
    internal static extern UIntPtr OpenDirDialog(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? title,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? initialDirectory,
        out IntPtr path);
#endif

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_event_type")]
    internal static extern IntPtr GetWindowEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_request_close_event_type")]
    internal static extern IntPtr GetWindowRequestCloseEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_closed_event_type")]
    internal static extern IntPtr GetWindowClosedEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_input_focus_event_type")]
    internal static extern IntPtr GetWindowInputFocusEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_lose_input_focus_event_type")]
    internal static extern IntPtr GetWindowLoseInputFocusEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_show_event_type")]
    internal static extern IntPtr GetWindowShowEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_hide_event_type")]
    internal static extern IntPtr GetWindowHideEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_resize_event_type")]
    internal static extern IntPtr GetWindowResizeEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_framebuffer_resize_event_type")]
    internal static extern IntPtr GetWindowFramebufferResizeEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_move_event_type")]
    internal static extern IntPtr GetWindowMoveEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_dpi_scale_changed_event_type")]
    internal static extern IntPtr GetWindowDpiScaleChangedEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_key_down_event_type")]
    internal static extern IntPtr GetWindowKeyDownEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_key_up_event_type")]
    internal static extern IntPtr GetWindowKeyUpEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_input_text_event_type")]
    internal static extern IntPtr GetWindowInputTextEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_mouse_enter_event_type")]
    internal static extern IntPtr GetWindowMouseEnterEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_mouse_leave_event_type")]
    internal static extern IntPtr GetWindowMouseLeaveEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_mouse_move_event_type")]
    internal static extern IntPtr GetWindowMouseMoveEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_mouse_down_event_type")]
    internal static extern IntPtr GetWindowMouseDownEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_mouse_up_event_type")]
    internal static extern IntPtr GetWindowMouseUpEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_scroll_event_type")]
    internal static extern IntPtr GetWindowScrollEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_touch_down_event_type")]
    internal static extern IntPtr GetWindowTouchDownEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_touch_move_event_type")]
    internal static extern IntPtr GetWindowTouchMoveEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_touch_up_event_type")]
    internal static extern IntPtr GetWindowTouchUpEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_window_drop_files_event_type")]
    internal static extern IntPtr GetWindowDropFilesEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_event_type")]
    internal static extern IntPtr GetApplicationEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_did_enter_foreground_event_type")]
    internal static extern IntPtr GetApplicationDidEnterForegroundEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_will_enter_foreground_event_type")]
    internal static extern IntPtr GetApplicationWillEnterForegroundEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_did_enter_background_event_type")]
    internal static extern IntPtr GetApplicationDidEnterBackgroundEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_will_enter_background_event_type")]
    internal static extern IntPtr GetApplicationWillEnterBackgroundEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_will_terminate_event_type")]
    internal static extern IntPtr GetApplicationWillTerminateEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_application_did_receive_memory_warning_event_type")]
    internal static extern IntPtr GetApplicationDidReceiveMemoryWarningEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_screen_keyboard_shown_event_type")]
    internal static extern IntPtr GetScreenKeyboardShownEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_get_screen_keyboard_hidden_event_type")]
    internal static extern IntPtr GetScreenKeyboardHiddenEventType();

    [DllImport(LibraryName, EntryPoint = "luna_window_event_get_window")]
    internal static extern UIntPtr WindowEventGetWindow(IntPtr eventObject, out NativeWindowHandle outWindow);

    [DllImport(LibraryName, EntryPoint = "luna_window_request_close_event_get_do_close")]
    internal static extern int WindowRequestCloseEventGetDoClose(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_request_close_event_set_do_close")]
    internal static extern void WindowRequestCloseEventSetDoClose(IntPtr eventObject, int doClose);

    [DllImport(LibraryName, EntryPoint = "luna_window_resize_event_get_size")]
    internal static extern void WindowResizeEventGetSize(IntPtr eventObject, out uint width, out uint height);

    [DllImport(LibraryName, EntryPoint = "luna_window_framebuffer_resize_event_get_size")]
    internal static extern void WindowFramebufferResizeEventGetSize(IntPtr eventObject, out uint width, out uint height);

    [DllImport(LibraryName, EntryPoint = "luna_window_move_event_get_position")]
    internal static extern void WindowMoveEventGetPosition(IntPtr eventObject, out int x, out int y);

    [DllImport(LibraryName, EntryPoint = "luna_window_key_down_event_get_key")]
    internal static extern uint WindowKeyDownEventGetKey(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_key_up_event_get_key")]
    internal static extern uint WindowKeyUpEventGetKey(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_input_text_event_get_text")]
    internal static extern IntPtr WindowInputTextEventGetText(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_mouse_move_event_get_position")]
    internal static extern void WindowMouseMoveEventGetPosition(IntPtr eventObject, out int x, out int y);

    [DllImport(LibraryName, EntryPoint = "luna_window_mouse_down_event_get_button")]
    internal static extern uint WindowMouseDownEventGetButton(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_mouse_up_event_get_button")]
    internal static extern uint WindowMouseUpEventGetButton(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_scroll_event_get_delta")]
    internal static extern void WindowScrollEventGetDelta(IntPtr eventObject, out float x, out float y);

    [DllImport(LibraryName, EntryPoint = "luna_window_touch_down_event_get_point")]
    internal static extern void WindowTouchDownEventGetPoint(IntPtr eventObject, out ulong id, out float x, out float y);

    [DllImport(LibraryName, EntryPoint = "luna_window_touch_move_event_get_point")]
    internal static extern void WindowTouchMoveEventGetPoint(IntPtr eventObject, out ulong id, out float x, out float y);

    [DllImport(LibraryName, EntryPoint = "luna_window_touch_up_event_get_point")]
    internal static extern void WindowTouchUpEventGetPoint(IntPtr eventObject, out ulong id, out float x, out float y);

    [DllImport(LibraryName, EntryPoint = "luna_window_drop_files_event_get_file_count")]
    internal static extern ulong WindowDropFilesEventGetFileCount(IntPtr eventObject);

    [DllImport(LibraryName, EntryPoint = "luna_window_drop_files_event_get_file")]
    internal static extern IntPtr WindowDropFilesEventGetFile(IntPtr eventObject, ulong index);

    [DllImport(LibraryName, EntryPoint = "luna_window_drop_files_event_get_position")]
    internal static extern void WindowDropFilesEventGetPosition(IntPtr eventObject, out float x, out float y);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_close")]
    internal static extern void IWindowClose(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_is_closed")]
    internal static extern int IWindowIsClosed(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_has_input_focus")]
    internal static extern int IWindowHasInputFocus(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_has_mouse_focus")]
    internal static extern int IWindowHasMouseFocus(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_is_minimized")]
    internal static extern int IWindowIsMinimized(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_foreground")]
    internal static extern UIntPtr IWindowSetForeground(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_is_maximized")]
    internal static extern int IWindowIsMaximized(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_minimized")]
    internal static extern UIntPtr IWindowSetMinimized(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_maximized")]
    internal static extern UIntPtr IWindowSetMaximized(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_restored")]
    internal static extern UIntPtr IWindowSetRestored(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_is_hovered")]
    internal static extern int IWindowIsHovered(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_is_visible")]
    internal static extern int IWindowIsVisible(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_visible")]
    internal static extern UIntPtr IWindowSetVisible(IntPtr self, int visible);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_get_style")]
    internal static extern uint IWindowGetStyle(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_style")]
    internal static extern UIntPtr IWindowSetStyle(IntPtr self, uint style);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_title")]
    internal static extern UIntPtr IWindowSetTitle(
        IntPtr self,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string title);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_get_position")]
    internal static extern void IWindowGetPosition(IntPtr self, out int x, out int y);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_position")]
    internal static extern UIntPtr IWindowSetPosition(IntPtr self, int x, int y);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_get_size")]
    internal static extern void IWindowGetSize(IntPtr self, out uint width, out uint height);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_size")]
    internal static extern UIntPtr IWindowSetSize(IntPtr self, uint width, uint height);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_get_framebuffer_size")]
    internal static extern void IWindowGetFramebufferSize(IntPtr self, out uint width, out uint height);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_get_dpi_scale_factor")]
    internal static extern float IWindowGetDpiScaleFactor(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_screen_to_client")]
    internal static extern void IWindowScreenToClient(IntPtr self, int x, int y, out int outX, out int outY);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_client_to_screen")]
    internal static extern void IWindowClientToScreen(IntPtr self, int x, int y, out int outX, out int outY);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_begin_text_input")]
    internal static extern UIntPtr IWindowBeginTextInput(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_set_text_input_area")]
    internal static extern UIntPtr IWindowSetTextInputArea(IntPtr self, in RectI inputRect, int cursor);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_end_text_input")]
    internal static extern UIntPtr IWindowEndTextInput(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_window_iwindow_is_text_input_active")]
    internal static extern int IWindowIsTextInputActive(IntPtr self);
}
