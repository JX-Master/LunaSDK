using System;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Numerics;
using Luna.Font;
using Luna.RHI;
using Luna.Runtime;
using Luna.VG;
using Luna.Window;

const uint HeaderTextHeight = 150;

Runtime.Init();

try
{
    WindowModule.Init("FontArrangeCSharpTest");
    Module.Init();
    FontModule.Init();
    VgModule.Init();

    using var device = Module.GetMainDevice();
    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = "Luna Font Arrange C# Test",
        Width = 1600,
        Height = 900
    });
    window.SetForeground();

    var graphicsQueue = FindGraphicsQueue(device);
    using var swapChain = device.CreateSwapChain(graphicsQueue, window, CreateSwapChainDesc(window.FramebufferSize));
    using var commandBuffer = device.CreateCommandBuffer(graphicsQueue);
    using var fontAtlas = VgModule.NewFontAtlas();
    using var drawList = VgModule.NewShapeDrawList(device);
    using var shapeRenderer = VgModule.NewFillShapeRenderer();
    using var defaultFont = FontModule.GetDefaultFont();

    TextArrangeResult? bodyArrangeResult = null;
    TextArrangeSection[] bodySections = Array.Empty<TextArrangeSection>();

    void RearrangeBodyText(Size2U framebufferSize)
    {
        if (framebufferSize.Width == 0 || framebufferSize.Height == 0)
        {
            return;
        }

        bodyArrangeResult?.Dispose();
        bodySections = new[]
        {
            new TextArrangeSection
            {
                FontFile = defaultFont,
                NumChars = (ulong)FontArrangeResources.BodyText.Length,
                FontIndex = 0,
                FontSize = 30.0f,
                Color = Vector4.One
            }
        };
        bodyArrangeResult = VgModule.ArrangeText(
            FontArrangeResources.BodyText,
            bodySections,
            new RectF(0.0f, 0.0f, framebufferSize.Width, MathF.Max(framebufferSize.Height - HeaderTextHeight, 0.0f)),
            TextAlignment.Center,
            TextAlignment.Center);
    }

    WindowModule.SetEventHandler(evt =>
    {
        if (!evt.IsA(WindowEventTypes.WindowFramebufferResizeEvent))
        {
            return;
        }

        using var eventWindow = WindowEvents.GetWindow(evt);
        if (eventWindow.GetNativeHandle() != window.GetNativeHandle())
        {
            return;
        }

        var framebufferSize = WindowEvents.GetFramebufferResizeSize(evt);
        if (framebufferSize.Width == 0 || framebufferSize.Height == 0)
        {
            return;
        }

        swapChain.Reset(CreateSwapChainDesc(framebufferSize));
        RearrangeBodyText(framebufferSize);
    });

    RearrangeBodyText(window.FramebufferSize);

    var lastFrameTimestamp = Stopwatch.GetTimestamp();
    var frameTimeMilliseconds = 16.0;

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

        var framebufferSize = window.FramebufferSize;
        if (framebufferSize.Width == 0 || framebufferSize.Height == 0)
        {
            Thread.Sleep(16);
            continue;
        }

        drawList.Reset();

        var fpsText = $"FPS: {(frameTimeMilliseconds > 0.0 ? 1000.0 / frameTimeMilliseconds : 0.0):F2}\n";
        var fpsSections = new[]
        {
            new TextArrangeSection
            {
                FontFile = defaultFont,
                NumChars = (ulong)fpsText.Length,
                FontIndex = 0,
                FontSize = 50.0f,
                Color = new Vector4(0.8f, 1.0f, 0.8f, 1.0f)
            }
        };
        using (var fpsArrangeResult = VgModule.ArrangeText(
            fpsText,
            fpsSections,
            new RectF(0.0f, framebufferSize.Height - HeaderTextHeight, framebufferSize.Width, HeaderTextHeight),
            TextAlignment.Center,
            TextAlignment.Center))
        {
            if (fpsArrangeResult.Lines.Length > 0)
            {
                VgModule.CommitTextArrangeResult(fpsArrangeResult, fpsSections, fontAtlas, drawList);
            }
        }

        if (bodyArrangeResult is not null && bodyArrangeResult.Lines.Length > 0)
        {
            VgModule.CommitTextArrangeResult(bodyArrangeResult, bodySections, fontAtlas, drawList);
        }

        drawList.Compile();

        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, new Color4(0.0f, 0.0f, 0.0f, 0.0f))
            }
        });
        commandBuffer.EndRenderPass();

        using var vertexBuffer = drawList.GetVertexBuffer() ?? throw new InvalidOperationException("VG draw list did not produce a vertex buffer.");
        using var indexBuffer = drawList.GetIndexBuffer() ?? throw new InvalidOperationException("VG draw list did not produce an index buffer.");
        var drawCalls = drawList.GetDrawCalls();

        shapeRenderer.Begin(backBuffer);
        shapeRenderer.Draw(vertexBuffer, indexBuffer, drawCalls);
        shapeRenderer.End();
        shapeRenderer.Submit(commandBuffer);

        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(backBuffer, new SubresourceIndex(0, 0), TextureStateFlags.Automatic, TextureStateFlags.Present)
        });
        commandBuffer.Submit(true);
        commandBuffer.Wait();
        swapChain.Present();
        commandBuffer.Reset();

        var now = Stopwatch.GetTimestamp();
        frameTimeMilliseconds = (now - lastFrameTimestamp) * 1000.0 / Stopwatch.Frequency;
        lastFrameTimestamp = now;
    }

    WindowModule.SetEventHandler(null);
    bodyArrangeResult?.Dispose();
}
finally
{
    Runtime.Close();
}

static uint FindGraphicsQueue(IDevice device)
{
    for (uint i = 0; i < device.CommandQueueCount; ++i)
    {
        if (device.GetCommandQueueDesc(i).Type == CommandQueueType.Graphics)
        {
            return i;
        }
    }
    throw new InvalidOperationException("No graphics queue is available on the main RHI device.");
}

static SwapChainDesc CreateSwapChainDesc(Size2U framebufferSize)
{
    return new SwapChainDesc(framebufferSize.Width, framebufferSize.Height, 2, Format.Bgra8Unorm, true);
}

static class FontArrangeResources
{
    private const string SampleText = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    public static readonly string BodyText = CreateBodyText();

    private static string CreateBodyText()
    {
        var builder = new StringBuilder();
        for (var i = 0; i < 300; ++i)
        {
            builder.Append(SampleText);
        }
        builder.Append('\n');
        return builder.ToString();
    }
}
