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
        RuntimeErrors.ThrowIfFailed(new ErrorCode(HidNativeGenerated.InitModule()));
    }

    public static bool SupportsKeyboard() => HidNativeGenerated.SupportsKeyboard() != 0;

    public static bool GetKeyState(KeyCode key) => HidNativeGenerated.GetKeyState((ushort)key) != 0;

    public static bool SupportsMouse() => HidNativeGenerated.SupportsMouse() != 0;

    public static bool GetMouseButtonState(MouseButton mouseButton) => HidNativeGenerated.GetMouseButtonState((byte)mouseButton) != 0;

    public static Point2U GetMousePosition() => HidNativeGenerated.GetMousePos();

    public static void SetMousePosition(int x, int y)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(HidNativeGenerated.SetMousePos(x, y)));
    }

    public static bool SupportsController() => HidNativeGenerated.SupportsController() != 0;

    public static ControllerInputState GetControllerState(uint index) => HidNativeGenerated.GetControllerState(index).ToPublic();

    public static void SetControllerState(uint index, ControllerOutputState state)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(HidNativeGenerated.SetControllerState(index, in state)));
    }
}
