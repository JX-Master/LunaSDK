using System;
using Luna.Runtime;
using Luna.Window;

Runtime.Init();

try
{
    WindowModule.Init("WindowCSharpTest");

    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = "Window C# Test",
        X = 100,
        Y = 100,
        Width = 320,
        Height = 240,
        CreationFlags = WindowCreationFlags.Hidden
    });

    if (window.IsClosed)
    {
        throw new InvalidOperationException("Newly created window should not be closed.");
    }

    window.SetTitle("Window C# Test Updated");
    _ = window.HasInputFocus;
    _ = window.HasMouseFocus;
    _ = window.IsMinimized;
    _ = window.IsMaximized;
    _ = window.IsHovered;
    _ = window.IsVisible;

    window.Position = new Point2I(120, 130);
    var clientPoint = new Point2I(3, 4);
    var screenPoint = window.ClientToScreen(clientPoint);
    var roundtripClientPoint = window.ScreenToClient(screenPoint);
    if (roundtripClientPoint.X != clientPoint.X || roundtripClientPoint.Y != clientPoint.Y)
    {
        throw new InvalidOperationException("Window coordinate conversion roundtrip failed.");
    }

    window.Size = new Size2U(333, 251);
    var resizedSize = window.Size;
    if (resizedSize.Width != 333 || resizedSize.Height != 251)
    {
        throw new InvalidOperationException("Window size update failed.");
    }

    var originalStyle = window.Style;
    window.Style = WindowStyleFlags.Resizable;
    if ((window.Style & WindowStyleFlags.Resizable) == 0)
    {
        throw new InvalidOperationException("Window style update failed.");
    }
    window.Style = originalStyle;

    window.BeginTextInput();
    window.SetTextInputArea(new RectI(0, 0, 100, 24), 4);
    if (!window.IsTextInputActive)
    {
        throw new InvalidOperationException("Text input should be active after BeginTextInput.");
    }
    window.EndTextInput();
    if (window.IsTextInputActive)
    {
        throw new InvalidOperationException("Text input should be inactive after EndTextInput.");
    }

    var clipboardText = $"Luna Window clipboard smoke {System.Guid.NewGuid():N}";
    string? oldClipboardText = null;
    var clipboardAvailable = true;
    try
    {
        oldClipboardText = WindowClipboard.Text;
        WindowClipboard.Text = clipboardText;
        if (WindowClipboard.Text != clipboardText)
        {
            throw new InvalidOperationException("Clipboard text roundtrip failed.");
        }
    }
    catch (ErrorException ex) when (ex.CodeName == "not_supported" || ex.CodeName == "bad_platform_call")
    {
        clipboardAvailable = false;
    }
    finally
    {
        if (clipboardAvailable && oldClipboardText is not null)
        {
            WindowClipboard.Text = oldClipboardText;
        }
    }

#if LUNA_PLATFORM_DESKTOP
    var primaryDisplay = WindowDisplays.PrimaryDisplay;
    if (primaryDisplay.IsValid)
    {
        var displays = WindowDisplays.GetDisplays();
        if (displays.Length == 0)
        {
            throw new InvalidOperationException("At least one display should be available.");
        }
        var displayMode = WindowDisplays.GetVideoMode(primaryDisplay);
        if (displayMode.Width == 0 || displayMode.Height == 0)
        {
            throw new InvalidOperationException("Primary display mode should be non-zero.");
        }
        _ = WindowDisplays.GetSupportedVideoModes(primaryDisplay);
        _ = WindowDisplays.GetPosition(primaryDisplay);
        var workingArea = WindowDisplays.GetWorkingArea(primaryDisplay);
        if (workingArea.Width <= 0 || workingArea.Height <= 0)
        {
            throw new InvalidOperationException("Primary display working area should be non-zero.");
        }
        if (string.IsNullOrWhiteSpace(WindowDisplays.GetName(primaryDisplay)))
        {
            throw new InvalidOperationException("Primary display name should not be empty.");
        }
    }
#endif

    var size = window.Size;
    if (size.Width == 0 || size.Height == 0)
    {
        throw new InvalidOperationException("Window size should be non-zero.");
    }

    var windowBaseType = WindowEventTypes.WindowEvent;
    if (!windowBaseType.IsValid || windowBaseType.Name != "Window::WindowEvent")
    {
        throw new InvalidOperationException("Window event base type lookup failed.");
    }
    if (!WindowEventTypes.WindowClosedEvent.IsA(windowBaseType))
    {
        throw new InvalidOperationException("Window closed event should be assignable to WindowEvent.");
    }

    var objectType = window.Type;
    if (!objectType.IsValid)
    {
        throw new InvalidOperationException("Window object type lookup failed.");
    }
    if (!window.IsA(objectType))
    {
        throw new InvalidOperationException("Window object should be assignable to its dynamic type.");
    }

    var invalidType = RuntimeTypes.GetByGuid(default);
    if (invalidType.IsValid || window.IsA(invalidType))
    {
        throw new InvalidOperationException("Invalid type handle should not match objects.");
    }

    var sawWindowEvent = false;
    var sawWindowClosedEvent = false;
    WindowModule.SetEventHandler(evt =>
    {
        if (evt.IsA(WindowEventTypes.WindowEvent))
        {
            sawWindowEvent = true;
            using var eventWindow = WindowEvents.GetWindow(evt);
            if (!eventWindow.Type.IsValid)
            {
                throw new InvalidOperationException("Window event should expose its target window.");
            }
        }
        if (evt.IsA(WindowEventTypes.WindowClosedEvent))
        {
            sawWindowClosedEvent = true;
        }
    });

    WindowModule.PollEvents();
    window.Close();
    WindowModule.PollEvents();
    WindowModule.SetEventHandler(null);

    if (!window.IsClosed)
    {
        throw new InvalidOperationException("Window should be closed after Close().");
    }
    if (!sawWindowEvent || !sawWindowClosedEvent)
    {
        throw new InvalidOperationException("Window close should dispatch a typed WindowClosedEvent.");
    }

    Console.WriteLine("WindowCSharpTest passed.");
}
finally
{
    Runtime.Close();
}
