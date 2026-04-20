using System;
using Luna.Runtime;
using Luna.Window;

var smoke = args.Length > 0 && Array.Exists(args, static arg => arg == "--smoke");
var running = true;
var frameCount = 0;
var lastMouseLogTicks = 0L;

Runtime.Init();

try
{
    WindowModule.Init("HelloWindow");

    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = smoke ? "HelloWindow Smoke" : "HelloWindow",
        X = smoke ? 100 : WindowModule.DefaultPosition,
        Y = smoke ? 100 : WindowModule.DefaultPosition,
        Width = 960,
        Height = 540,
        CreationFlags = smoke ? WindowCreationFlags.Hidden : WindowCreationFlags.None
    });

    WindowModule.SetEventHandler(evt =>
    {
        if (evt.IsA(WindowEventTypes.WindowRequestCloseEvent))
        {
            WindowEvents.SetDoClose(evt, true);
            Console.WriteLine("request close");
        }
        else if (evt.IsA(WindowEventTypes.WindowClosedEvent))
        {
            Console.WriteLine("closed");
            running = false;
        }
        else if (evt.IsA(WindowEventTypes.WindowResizeEvent))
        {
            var size = WindowEvents.GetResizeSize(evt);
            Console.WriteLine($"resize {size.Width}x{size.Height}");
        }
        else if (evt.IsA(WindowEventTypes.WindowFramebufferResizeEvent))
        {
            var size = WindowEvents.GetFramebufferResizeSize(evt);
            Console.WriteLine($"framebuffer {size.Width}x{size.Height}");
        }
        else if (evt.IsA(WindowEventTypes.WindowMoveEvent))
        {
            var pos = WindowEvents.GetMovePosition(evt);
            Console.WriteLine($"move {pos.X},{pos.Y}");
        }
        else if (evt.IsA(WindowEventTypes.WindowKeyDownEvent))
        {
            var key = WindowEvents.GetKeyDownKey(evt);
            Console.WriteLine($"key down {key}");
            if (key == 1)
            {
                window.Close();
            }
        }
        else if (evt.IsA(WindowEventTypes.WindowKeyUpEvent))
        {
            Console.WriteLine($"key up {WindowEvents.GetKeyUpKey(evt)}");
        }
        else if (evt.IsA(WindowEventTypes.WindowInputTextEvent))
        {
            Console.WriteLine($"text {WindowEvents.GetInputText(evt)}");
        }
        else if (evt.IsA(WindowEventTypes.WindowMouseMoveEvent))
        {
            var now = Environment.TickCount64;
            if (now - lastMouseLogTicks > 250)
            {
                lastMouseLogTicks = now;
                var pos = WindowEvents.GetMouseMovePosition(evt);
                Console.WriteLine($"mouse move {pos.X},{pos.Y}");
            }
        }
        else if (evt.IsA(WindowEventTypes.WindowMouseDownEvent))
        {
            Console.WriteLine($"mouse down {WindowEvents.GetMouseDownButton(evt)}");
        }
        else if (evt.IsA(WindowEventTypes.WindowMouseUpEvent))
        {
            Console.WriteLine($"mouse up {WindowEvents.GetMouseUpButton(evt)}");
        }
        else if (evt.IsA(WindowEventTypes.WindowScrollEvent))
        {
            var delta = WindowEvents.GetScrollDelta(evt);
            Console.WriteLine($"scroll {delta.X:0.###},{delta.Y:0.###}");
        }
    });

    Console.WriteLine(smoke ? "HelloWindow smoke started." : "HelloWindow started. Close the window or press Esc to exit.");

    while (running)
    {
        WindowModule.PollEvents();
        if (smoke && ++frameCount >= 2)
        {
            window.Close();
        }
    }

    WindowModule.SetEventHandler(null);
}
finally
{
    Runtime.Close();
}
