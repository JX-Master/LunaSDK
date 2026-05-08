using System;
using Luna.HID.Internal;
using Luna.Runtime;

namespace Luna.HID;

public static class Module
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the HID module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(HidNative.InitModule()));
    }

    public static bool SupportsKeyboard() => HidNative.SupportsKeyboard() != 0;

    public static bool GetKeyState(KeyCode key) => HidNative.GetKeyState((ushort)key) != 0;

    public static bool SupportsMouse() => HidNative.SupportsMouse() != 0;

    public static bool GetMouseButtonState(MouseButton mouseButton) => HidNative.GetMouseButtonState((byte)mouseButton) != 0;

    public static Point2U GetMousePosition() => HidNative.GetMousePosition();

    public static void SetMousePosition(int x, int y)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(HidNative.SetMousePosition(x, y)));
    }

    public static bool SupportsController() => HidNative.SupportsController() != 0;

    public static ControllerInputState GetControllerState(uint index) => HidNative.GetControllerState(index).ToPublic();

    public static void SetControllerState(uint index, ControllerOutputState state)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(HidNative.SetControllerState(index, in state)));
    }
}
