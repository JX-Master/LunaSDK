using System;
using System.Numerics;
using System.Threading;
using Luna.ImGui;
using Luna.RHI;
using Luna.Runtime;
using Luna.Window;
using RhiModule = Luna.RHI.Module;
using RhiUtilityModule = Luna.RHIUtility.Module;

Runtime.Init();

try
{
    WindowModule.Init("ImGuiCSharpTest");
    RhiModule.Init();
    RhiUtilityModule.Init();
    ImGuiModule.Init();

    using var device = RhiModule.GetMainDevice();
    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = "ImGui Full API C#",
        Width = 1680,
        Height = 960
    });
    window.SetForeground();

    var graphicsQueue = FindGraphicsQueue(device);
    using var swapChain = device.CreateSwapChain(graphicsQueue, window, CreateSwapChainDesc(window.FramebufferSize));
    using var commandBuffer = device.CreateCommandBuffer(graphicsQueue);
    using var demoTexture = CreateDemoTexture(device, commandBuffer);
    using var sampledImage = ImGuiModule.NewSampledImage(demoTexture, CreateLinearClampSampler());

    ImGuiModule.SetActiveWindow(window);
    WindowModule.SetEventHandler(evt => ImGuiModule.HandleWindowEvent(evt));

    var glyphRanges = ImGuiModule.GetGlyphRangesDefault();
    var singleLineText = "Hello Luna ImGui";
    var hintedText = string.Empty;
    var multilineText = "This multiline editor is coming from Luna.ImGui in C#.\nTry typing, resizing the window, and dragging the gizmo on the right.";
    var imageButtonClicks = 0;
    var worldMatrix = Matrix4x4.Identity;
    var viewMatrix = Matrix4x4.CreateLookAt(new Vector3(0.0f, 0.0f, -4.0f), Vector3.Zero, Vector3.UnitY);
    var projectionMatrix = Matrix4x4.CreatePerspectiveFieldOfView(MathF.PI / 3.0f, 1.0f, 0.1f, 100.0f);

    InputTextCallback uppercaseFilter = data =>
    {
        if (data.EventFlag == InputTextFlags.CallbackCharFilter && data.EventChar is >= (uint)'a' and <= (uint)'z')
        {
            data.EventChar -= (uint)('a' - 'A');
        }
        return 0;
    };

    InputTextCallback clampMultilineLength = data =>
    {
        if (data.EventFlag == InputTextFlags.CallbackEdit && data.Text.Length > 160)
        {
            data.Text = data.Text[..160];
            data.CursorPos = Math.Min(data.CursorPos, data.Text.Length);
        }
        return 0;
    };

    uint width = 0;
    uint height = 0;

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

        if (framebufferSize.Width != width || framebufferSize.Height != height)
        {
            swapChain.Reset(new SwapChainDesc(framebufferSize.Width, framebufferSize.Height, 2, Format.Unknown, true));
            width = framebufferSize.Width;
            height = framebufferSize.Height;
        }

        ImGuiModule.UpdateIo();
        ImGuiApi.NewFrame();
        ImGuiApi.ShowDemoWindow();

        if (ImGuiApi.Begin("Managed ImGui Surface"))
        {
            var windowSize = window.Size;
            ImGuiApi.Text($"Window Size: {windowSize.Width}x{windowSize.Height}");
            ImGuiApi.Text($"Framebuffer Size: {framebufferSize.Width}x{framebufferSize.Height}");
            ImGuiApi.Text($"DPI Scale: {window.DpiScaleFactor:F3}");
            ImGuiApi.Text($"Default Glyph Ranges: {glyphRanges.Length}");
            ImGuiApi.Text($"Image Button Clicks: {imageButtonClicks}");

            ImGuiApi.InputText(
                "Uppercase Filter",
                ref singleLineText,
                InputTextFlags.CallbackCharFilter | InputTextFlags.AutoSelectAll,
                uppercaseFilter);

            ImGuiApi.InputTextWithHint(
                "With Hint",
                "Type something here",
                ref hintedText,
                InputTextFlags.EscapeClearsAll);

            ImGuiApi.InputTextMultiline(
                "Multiline",
                ref multilineText,
                new Vector2(520.0f, 140.0f),
                InputTextFlags.CallbackEdit | InputTextFlags.WordWrap,
                clampMultilineLength);

            ImGuiApi.Text("Texture");
            ImGuiApi.Image(demoTexture, new Vector2(128.0f, 128.0f));
            ImGuiApi.Text("Sampled Image");
            ImGuiApi.Image(sampledImage, new Vector2(128.0f, 128.0f), Vector2.Zero, Vector2.One);
            if (ImGuiApi.ImageButton("SampledImageButton", sampledImage, new Vector2(96.0f, 96.0f)))
            {
                ++imageButtonClicks;
            }

            var viewportWidth = MathF.Max(240.0f, framebufferSize.Width * 0.4f);
            var viewportHeight = MathF.Max(240.0f, framebufferSize.Height * 0.45f);
            var viewportRect = new Luna.ImGui.RectF(
                framebufferSize.Width - viewportWidth - 40.0f,
                120.0f,
                viewportWidth,
                viewportHeight);
            ImGuiApi.Gizmo(
                ref worldMatrix,
                in viewMatrix,
                in projectionMatrix,
                viewportRect,
                GizmoOperation.Translate,
                GizmoMode.World,
                0.25f,
                enabled: true,
                orthographic: false,
                out var deltaMatrix,
                out var isMouseHover,
                out var isMouseMoving);
            ImGuiApi.Text($"Gizmo Hover: {isMouseHover}");
            ImGuiApi.Text($"Gizmo Moving: {isMouseMoving}");
            ImGuiApi.Text($"World Translation: {worldMatrix.M41:F2}, {worldMatrix.M42:F2}, {worldMatrix.M43:F2}");
            ImGuiApi.Text($"Delta Translation: {deltaMatrix.M41:F2}, {deltaMatrix.M42:F2}, {deltaMatrix.M43:F2}");
        }
        ImGuiApi.End();

        ImGuiApi.Render();

        using var backBuffer = swapChain.GetCurrentBackBuffer();
        commandBuffer.BeginRenderPass(new RenderPassDesc
        {
            ColorAttachments = new[]
            {
                new ColorAttachment(backBuffer, LoadOp.Clear, StoreOp.Store, new Color4(0.07f, 0.08f, 0.10f, 1.0f))
            }
        });
        commandBuffer.EndRenderPass();
        ImGuiModule.RenderDrawData(commandBuffer, backBuffer);
        commandBuffer.ResourceBarrier(new[]
        {
            new TextureBarrier(backBuffer, new SubresourceIndex(0, 0), TextureStateFlags.Automatic, TextureStateFlags.Present)
        });
        commandBuffer.Submit(true);
        commandBuffer.Wait();
        swapChain.Present();
        commandBuffer.Reset();
    }

    WindowModule.SetEventHandler(null);
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

static ITexture CreateDemoTexture(IDevice device, ICommandBuffer commandBuffer)
{
    var texture = device.CreateTexture(
        MemoryType.Local,
        new TextureDesc(
            TextureType.Tex2D,
            Format.Rgba8Unorm,
            4,
            4,
            1,
            1,
            1,
            1,
            TextureUsageFlags.CopyDestination | TextureUsageFlags.ReadTexture,
            ResourceFlags.None));

    var pixels = new byte[]
    {
        255,  64,  64, 255,   64, 255,  64, 255,   64,  64, 255, 255,  255, 255,  64, 255,
         64, 255, 255, 255,  255, 128,  64, 255,  255,  64, 255, 255,  128, 255, 128, 255,
        255, 255, 255, 255,   32,  32,  32, 255,  192, 192, 192, 255,   96,  96, 255, 255,
        255, 128, 192, 255,  128, 192, 255, 255,  255, 192, 128, 255,    0,   0,   0, 255
    };

    using var writer = RhiUtilityModule.CreateResourceWriteContext(device);
    writer.Reset();
    writer.WriteTexture(texture, new SubresourceIndex(0, 0), 0, 0, 0, 4, 4, 1, pixels, 16, 64, 16);
    writer.Commit(commandBuffer, submitAndWait: true);
    commandBuffer.Reset();
    return texture;
}

static SamplerDesc CreateLinearClampSampler()
{
    return new SamplerDesc(
        Filter.Linear,
        Filter.Linear,
        Filter.Linear,
        TextureAddressMode.Clamp,
        TextureAddressMode.Clamp,
        TextureAddressMode.Clamp);
}
