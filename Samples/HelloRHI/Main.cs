using System;
using System.Threading;
using Luna.Runtime;
using Luna.RHI;
using Luna.Window;

var smoke = args.Length > 0 && Array.Exists(args, static arg => arg == "--smoke");
var running = true;
var frameCount = 0;

Runtime.Init();

try
{
    WindowModule.Init("HelloRHI");
    try
    {
        Module.Init();
    }
    catch (ErrorException ex) when (smoke)
    {
        Console.WriteLine($"HelloRHI smoke skipped: {ex.Message}");
        return;
    }

    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = $"HelloRHI ({Module.BackendType})",
        X = smoke ? 100 : WindowModule.DefaultPosition,
        Y = smoke ? 100 : WindowModule.DefaultPosition,
        Width = 960,
        Height = 540,
        CreationFlags = smoke ? WindowCreationFlags.Hidden : WindowCreationFlags.None
    });

    using var device = Module.GetMainDevice();
    var commandQueue = uint.MaxValue;
    Console.WriteLine($"RHI backend: {Module.BackendType}, queues: {device.CommandQueueCount}");
    for (uint i = 0; i < device.CommandQueueCount; ++i)
    {
        var queue = device.GetCommandQueueDesc(i);
        Console.WriteLine($"  Queue {i}: {queue.Type}, {queue.Flags}");
        if (queue.Type == CommandQueueType.Graphics && queue.Flags.HasFlag(CommandQueueFlags.Presenting))
        {
            commandQueue = i;
        }
    }
    if (commandQueue == uint.MaxValue)
    {
        throw new NotSupportedException("No graphics command queue supports presenting.");
    }

    using var swapChain = device.CreateSwapChain(commandQueue, window, new SwapChainDesc(
        0,
        0,
        2,
        Format.Bgra8Unorm,
        verticalSynchronized: true));
    using var commandBuffer = device.CreateCommandBuffer(commandQueue);

    WindowModule.SetEventHandler(evt =>
    {
        if (evt.IsA(WindowEventTypes.WindowClosedEvent))
        {
            running = false;
        }
        else if (evt.IsA(WindowEventTypes.WindowFramebufferResizeEvent))
        {
            var size = WindowEvents.GetFramebufferResizeSize(evt);
            if (size.Width != 0 && size.Height != 0)
            {
                var desc = swapChain.Desc;
                swapChain.Reset(new SwapChainDesc(
                    size.Width,
                    size.Height,
                    desc.BufferCount,
                    desc.Format,
                    desc.VerticalSynchronized,
                    desc.ColorSpace));
            }
        }
        else if (evt.IsA(WindowEventTypes.WindowKeyDownEvent) && WindowEvents.GetKeyDownKey(evt) == 1)
        {
            window.Close();
        }
    });

    Console.WriteLine(smoke ? "HelloRHI smoke started." : "HelloRHI started. Close the window or press Esc to exit.");

    while (running)
    {
        WindowModule.PollEvents();
        if (window.IsMinimized)
        {
            Thread.Sleep(100);
            continue;
        }

        var t = frameCount * 0.025f;
        var clear = new Color4(
            0.12f + 0.08f * MathF.Sin(t),
            0.18f + 0.08f * MathF.Sin(t + 2.0f),
            0.32f + 0.12f * MathF.Sin(t + 4.0f),
            1.0f);

        commandBuffer.Reset();
        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.ColorAttachmentWrite,
                ResourceBarrierFlags.DiscardContent)
        });
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, clear)
            }
        });
        commandBuffer.EndRenderPass();
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(
                backBuffer,
                SubresourceIndex.AllSubresources,
                TextureStateFlags.Automatic,
                TextureStateFlags.Present)
        });
        commandBuffer.Submit(allowHostWaiting: true);
        commandBuffer.Wait();
        swapChain.Present();

        if (smoke && ++frameCount >= 2)
        {
            window.Close();
        }
        else if (!smoke)
        {
            ++frameCount;
        }
    }

    WindowModule.SetEventHandler(null);
}
finally
{
    Runtime.Close();
}
