using System;
using System.Runtime.InteropServices;
using System.Threading;
using Luna.RHI;
using Luna.Runtime;
using Luna.Window;
using RuntimeCore = Luna.Runtime.Runtime;

namespace Luna.Samples.ManagedHostApp;

public static class Program
{
    public static int Main(string[] args)
    {
        return ManagedApp.Run(args);
    }
}

public static class ManagedEntry
{
    [UnmanagedCallersOnly]
    public static int Run(IntPtr args, int argsSize)
    {
        try
        {
            return ManagedApp.Run(NativeHostArguments.Read(args, argsSize));
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);
            return -1;
        }
    }
}

internal static class ManagedApp
{
    public static int Run(string[] args)
    {
        var smoke = Array.Exists(args, static arg => arg == "--smoke");
        var requireRhi = Array.Exists(args, static arg => arg == "--require-rhi");
        var running = true;
        var frameCount = 0;

        RuntimeCore.Init();

        try
        {
            WindowModule.Init("ManagedHostApp");

            using var window = WindowModule.CreateWindow(new WindowCreationDesc
            {
                Title = "ManagedHostApp",
                X = smoke ? 100 : WindowModule.DefaultPosition,
                Y = smoke ? 100 : WindowModule.DefaultPosition,
                Width = 960,
                Height = 540,
                CreationFlags = smoke ? WindowCreationFlags.Hidden : WindowCreationFlags.None
            });

            if (!smoke)
            {
                window.SetForeground();
            }

            IDevice? device = null;
            ISwapChain? swapChain = null;
            ICommandBuffer? commandBuffer = null;
            try
            {
                Module.Init();
                device = Module.GetMainDevice();
                var commandQueue = FindPresentQueue(device);
                swapChain = device.CreateSwapChain(commandQueue, window, new SwapChainDesc(
                    0,
                    0,
                    2,
                    Format.Bgra8Unorm,
                    verticalSynchronized: true));
                commandBuffer = device.CreateCommandBuffer(commandQueue);
                Console.WriteLine($"ManagedHostApp RHI ready: {Module.BackendType}");
            }
            catch (ErrorException ex) when (!requireRhi)
            {
                commandBuffer?.Dispose();
                commandBuffer = null;
                swapChain?.Dispose();
                swapChain = null;
                device?.Dispose();
                device = null;
                Console.WriteLine($"ManagedHostApp RHI skipped: {ex.Message}");
            }

            try
            {
                try
                {
                    WindowModule.SetEventHandler(evt =>
                    {
                        if (evt.IsA(WindowEventTypes.WindowClosedEvent))
                        {
                            running = false;
                        }
                        else if (evt.IsA(WindowEventTypes.WindowRequestCloseEvent))
                        {
                            WindowEvents.SetDoClose(evt, true);
                        }
                        else if (swapChain is not null && evt.IsA(WindowEventTypes.WindowFramebufferResizeEvent))
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

                    Console.WriteLine(smoke ? "ManagedHostApp smoke started." : "ManagedHostApp started. Close the window or press Esc to exit.");

                    while (running)
                    {
                        WindowModule.PollEvents();

                        if (commandBuffer is not null && swapChain is not null && !window.IsMinimized)
                        {
                            var t = frameCount * 0.025f;
                            RenderFrame(commandBuffer, swapChain, new Color4(
                                0.10f + 0.06f * MathF.Sin(t),
                                0.18f + 0.07f * MathF.Sin(t + 2.0f),
                                0.28f + 0.10f * MathF.Sin(t + 4.0f),
                                1.0f));
                        }
                        else
                        {
                            Thread.Sleep(16);
                        }

                        if (smoke && ++frameCount >= 2)
                        {
                            window.Close();
                        }
                        else if (!smoke)
                        {
                            ++frameCount;
                        }
                    }
                }
                finally
                {
                    WindowModule.SetEventHandler(null);
                }
            }
            finally
            {
                commandBuffer?.Dispose();
                swapChain?.Dispose();
                device?.Dispose();
            }
        }
        finally
        {
            RuntimeCore.Close();
        }

        return 0;
    }

    private static uint FindPresentQueue(IDevice device)
    {
        Console.WriteLine($"RHI backend: {Module.BackendType}, queues: {device.CommandQueueCount}");
        for (uint i = 0; i < device.CommandQueueCount; ++i)
        {
            var queue = device.GetCommandQueueDesc(i);
            Console.WriteLine($"  Queue {i}: {queue.Type}, {queue.Flags}");
            if (queue.Type == CommandQueueType.Graphics && queue.Flags.HasFlag(CommandQueueFlags.Presenting))
            {
                return i;
            }
        }

        throw new NotSupportedException("No graphics command queue supports presenting.");
    }

    private static void RenderFrame(ICommandBuffer commandBuffer, ISwapChain swapChain, Color4 clear)
    {
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
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeHostArguments
{
    private readonly int _count;
    private readonly IntPtr _values;

    public static string[] Read(IntPtr args, int argsSize)
    {
        if (args == IntPtr.Zero || argsSize < Marshal.SizeOf<NativeHostArguments>())
        {
            return Array.Empty<string>();
        }

        var nativeArgs = Marshal.PtrToStructure<NativeHostArguments>(args);
        if (nativeArgs._count <= 0 || nativeArgs._values == IntPtr.Zero)
        {
            return Array.Empty<string>();
        }

        var result = new string[nativeArgs._count];
        for (var i = 0; i < result.Length; ++i)
        {
            var valuePtr = Marshal.ReadIntPtr(nativeArgs._values, i * IntPtr.Size);
            result[i] = Marshal.PtrToStringUTF8(valuePtr) ?? string.Empty;
        }
        return result;
    }
}
