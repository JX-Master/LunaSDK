using System;
using System.Collections.Generic;
using System.Threading;
using Luna.HID;
using Luna.Runtime;
using Luna.Window;
using HidModule = Luna.HID.Module;

Runtime.Init();

var originalMousePosition = default(Point2U);
var restoredMousePosition = false;

try
{
    WindowModule.Init("HIDCSharpTest");
    HidModule.Init();

    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = "HID C# Test",
        Width = 960,
        Height = 540
    });
    window.SetForeground();

    var supportsKeyboard = HidModule.SupportsKeyboard();
    var supportsMouse = HidModule.SupportsMouse();
    var supportsController = HidModule.SupportsController();

    if (supportsMouse)
    {
        originalMousePosition = HidModule.GetMousePosition();
        HidModule.SetMousePosition((int)originalMousePosition.X, (int)originalMousePosition.Y);
        restoredMousePosition = true;
    }

    if (supportsController)
    {
        var controllerState = HidModule.GetControllerState(0);
        if (controllerState.Connected)
        {
            HidModule.SetControllerState(0, new ControllerOutputState(0.0f, 0.0f));
        }
    }

    var lastKeyDown = KeyCode.Unknown;
    var lastKeyUp = KeyCode.Unknown;
    var lastMouseDown = MouseButton.None;
    var lastMouseUp = MouseButton.None;
    string lastInputText = string.Empty;
    var pressedKeys = new HashSet<KeyCode>();
    uint keyDownCount = 0;
    uint mouseDownCount = 0;

    WindowModule.SetEventHandler(evt =>
    {
        if (evt.IsA(WindowEventTypes.WindowKeyDownEvent))
        {
            lastKeyDown = (KeyCode)WindowEvents.GetKeyDownKey(evt);
            pressedKeys.Add(lastKeyDown);
            ++keyDownCount;
        }
        else if (evt.IsA(WindowEventTypes.WindowKeyUpEvent))
        {
            lastKeyUp = (KeyCode)WindowEvents.GetKeyUpKey(evt);
            pressedKeys.Remove(lastKeyUp);
        }
        else if (evt.IsA(WindowEventTypes.WindowMouseDownEvent))
        {
            lastMouseDown = (MouseButton)WindowEvents.GetMouseDownButton(evt);
            ++mouseDownCount;
        }
        else if (evt.IsA(WindowEventTypes.WindowMouseUpEvent))
        {
            lastMouseUp = (MouseButton)WindowEvents.GetMouseUpButton(evt);
        }
        else if (evt.IsA(WindowEventTypes.WindowInputTextEvent))
        {
            lastInputText = WindowEvents.GetInputText(evt);
        }
    });

    Console.WriteLine("HIDCSharpTest running. Close the window to finish.");
    Console.WriteLine("Suggested manual checks: press WASD/QE, click mouse buttons, move the cursor, and verify the title updates.");

    while (!window.IsClosed)
    {
        WindowModule.PollEvents();
        if (window.IsClosed)
        {
            break;
        }
        if (window.IsMinimized)
        {
            Thread.Sleep(100);
            continue;
        }

        var mousePosition = supportsMouse ? HidModule.GetMousePosition() : default;
        var wEvent = pressedKeys.Contains(KeyCode.W) ? 1 : 0;
        var aEvent = pressedKeys.Contains(KeyCode.A) ? 1 : 0;
        var sEvent = pressedKeys.Contains(KeyCode.S) ? 1 : 0;
        var dEvent = pressedKeys.Contains(KeyCode.D) ? 1 : 0;
        var qEvent = pressedKeys.Contains(KeyCode.Q) ? 1 : 0;
        var eEvent = pressedKeys.Contains(KeyCode.E) ? 1 : 0;
        var wPoll = supportsKeyboard && HidModule.GetKeyState(KeyCode.W) ? 1 : 0;
        var aPoll = supportsKeyboard && HidModule.GetKeyState(KeyCode.A) ? 1 : 0;
        var sPoll = supportsKeyboard && HidModule.GetKeyState(KeyCode.S) ? 1 : 0;
        var dPoll = supportsKeyboard && HidModule.GetKeyState(KeyCode.D) ? 1 : 0;
        var qPoll = supportsKeyboard && HidModule.GetKeyState(KeyCode.Q) ? 1 : 0;
        var ePoll = supportsKeyboard && HidModule.GetKeyState(KeyCode.E) ? 1 : 0;
        var rightMouse = supportsMouse && HidModule.GetMouseButtonState(MouseButton.Right) ? 1 : 0;
        var controllerState = supportsController ? HidModule.GetControllerState(0) : default;

        window.SetTitle(
            $"HID C# Test | K:{(supportsKeyboard ? "Y" : "N")} M:{(supportsMouse ? "Y" : "N")} C:{(supportsController ? "Y" : "N")} " +
            $"| Mouse:{mousePosition.X},{mousePosition.Y} RMB:{rightMouse} " +
            $"| Evt W:{wEvent} A:{aEvent} S:{sEvent} D:{dEvent} Q:{qEvent} E:{eEvent} " +
            $"| Poll W:{wPoll} A:{aPoll} S:{sPoll} D:{dPoll} Q:{qPoll} E:{ePoll} " +
            $"| LastKD:{lastKeyDown} LastKU:{lastKeyUp} LastMD:{lastMouseDown} LastMU:{lastMouseUp} " +
            $"| Ctrl0:{(controllerState.Connected ? 1 : 0)} Btn:{controllerState.Buttons}");

        Thread.Sleep(16);
    }

    WindowModule.SetEventHandler(null);

    if (keyDownCount == 0)
    {
        Console.WriteLine("HIDCSharpTest finished without observing any key-down events.");
    }
    if (mouseDownCount == 0)
    {
        Console.WriteLine("HIDCSharpTest finished without observing any mouse-down events.");
    }
    if (!string.IsNullOrEmpty(lastInputText))
    {
        Console.WriteLine($"Last input text: {lastInputText}");
    }
    Console.WriteLine("HIDCSharpTest passed.");
}
finally
{
    try
    {
        if (restoredMousePosition)
        {
            HidModule.SetMousePosition((int)originalMousePosition.X, (int)originalMousePosition.Y);
        }
    }
    catch
    {
    }

    Runtime.Close();
}
