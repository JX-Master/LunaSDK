using System;
using System.Runtime.InteropServices;

namespace Luna.HID.Internal;

internal static class HidNative
{
    private const string LibraryName = "LunaHIDC";

    [DllImport(LibraryName, EntryPoint = "luna_hid_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_hid_supports_keyboard")]
    internal static extern int SupportsKeyboard();

    [DllImport(LibraryName, EntryPoint = "luna_hid_get_key_state")]
    internal static extern int GetKeyState(ushort key);

    [DllImport(LibraryName, EntryPoint = "luna_hid_supports_mouse")]
    internal static extern int SupportsMouse();

    [DllImport(LibraryName, EntryPoint = "luna_hid_get_mouse_button_state")]
    internal static extern int GetMouseButtonState(byte mouseButton);

    [DllImport(LibraryName, EntryPoint = "luna_hid_get_mouse_pos")]
    internal static extern Point2U GetMousePosition();

    [DllImport(LibraryName, EntryPoint = "luna_hid_set_mouse_pos")]
    internal static extern UIntPtr SetMousePosition(int x, int y);

    [DllImport(LibraryName, EntryPoint = "luna_hid_supports_controller")]
    internal static extern int SupportsController();

    [DllImport(LibraryName, EntryPoint = "luna_hid_get_controller_state")]
    internal static extern NativeControllerInputState GetControllerState(uint index);

    [DllImport(LibraryName, EntryPoint = "luna_hid_set_controller_state")]
    internal static extern UIntPtr SetControllerState(uint index, in ControllerOutputState state);
}
