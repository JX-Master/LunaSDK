using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Numerics;
using System.Threading;
using Luna.Font;
using Luna.RHI;
using Luna.Runtime;
using Luna.VG;
using Luna.Window;

const uint MouseButtonRight = 0x02;
const uint KeyA = 28;
const uint KeyD = 31;
const uint KeyE = 32;
const uint KeyQ = 44;
const uint KeyS = 46;
const uint KeyW = 50;
const uint KeyLeftShift = 61;

Runtime.Init();

try
{
    WindowModule.Init("VGVisualCSharpTest");
    Module.Init();
    FontModule.Init();
    VgModule.Init();

    using var device = Module.GetMainDevice();
    using var window = WindowModule.CreateWindow(new WindowCreationDesc
    {
        Title = "Luna Vector Graphics C# Test",
        Width = 1600,
        Height = 1000
    });
    window.SetForeground();

    var framebufferSize = window.FramebufferSize;
    var graphicsQueue = FindGraphicsQueue(device);
    using var swapChain = device.CreateSwapChain(graphicsQueue, window, CreateSwapChainDesc(framebufferSize));
    using var commandBuffer = device.CreateCommandBuffer(graphicsQueue);
    using var fontAtlas = VgModule.NewFontAtlas();
    using var drawList = VgModule.NewShapeDrawList(device);
    using var shapeRenderer = VgModule.NewFillShapeRenderer();
    using var font = FontModule.GetDefaultFont();
    using var shapeBuffer = VgModule.NewShapeBuffer();

    var cameraPosition = new Vector3(framebufferSize.Width / 2.0f, framebufferSize.Height / 2.0f, -3000.0f);
    var yaw = 0.0f;
    var pitch = 0.0f;
    var cameraNavigating = false;
    var mousePosition = Vector2.Zero;
    var pressedKeys = new HashSet<uint>();

    WindowModule.SetEventHandler(evt =>
    {
        if (!evt.IsA(WindowEventTypes.WindowEvent))
        {
            return;
        }

        using var eventWindow = WindowEvents.GetWindow(evt);
        if (eventWindow.GetNativeHandle() != window.GetNativeHandle())
        {
            return;
        }

        if (evt.IsA(WindowEventTypes.WindowFramebufferResizeEvent))
        {
            var size = WindowEvents.GetFramebufferResizeSize(evt);
            if (size.Width != 0 && size.Height != 0)
            {
                swapChain.Reset(CreateSwapChainDesc(size));
            }
            return;
        }

        if (evt.IsA(WindowEventTypes.WindowKeyDownEvent))
        {
            pressedKeys.Add(WindowEvents.GetKeyDownKey(evt));
            return;
        }

        if (evt.IsA(WindowEventTypes.WindowKeyUpEvent))
        {
            pressedKeys.Remove(WindowEvents.GetKeyUpKey(evt));
            return;
        }

        if (evt.IsA(WindowEventTypes.WindowMouseMoveEvent))
        {
            var (x, y) = WindowEvents.GetMouseMovePosition(evt);
            var nextMousePosition = new Vector2(x, y);
            if (cameraNavigating)
            {
                var delta = nextMousePosition - mousePosition;
                pitch = Math.Clamp(pitch + DegreesToRadians(delta.Y / 10.0f), DegreesToRadians(-85.0f), DegreesToRadians(85.0f));
                yaw += DegreesToRadians(delta.X / 10.0f);
            }
            mousePosition = nextMousePosition;
            return;
        }

        if (evt.IsA(WindowEventTypes.WindowMouseDownEvent) && WindowEvents.GetMouseDownButton(evt) == MouseButtonRight)
        {
            cameraNavigating = true;
            return;
        }

        if (evt.IsA(WindowEventTypes.WindowMouseUpEvent) && WindowEvents.GetMouseUpButton(evt) == MouseButtonRight)
        {
            cameraNavigating = false;
        }
    });

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

        framebufferSize = window.FramebufferSize;
        if (framebufferSize.Width == 0 || framebufferSize.Height == 0)
        {
            Thread.Sleep(16);
            continue;
        }

        if (cameraNavigating)
        {
            var (forward, left, up) = ComputeCameraBasis(yaw, pitch);
            var cameraSpeed = pressedKeys.Contains(KeyLeftShift) ? 20.0f : 10.0f;
            if (pressedKeys.Contains(KeyW)) cameraPosition += forward * cameraSpeed;
            if (pressedKeys.Contains(KeyA)) cameraPosition += left * cameraSpeed;
            if (pressedKeys.Contains(KeyS)) cameraPosition -= forward * cameraSpeed;
            if (pressedKeys.Contains(KeyD)) cameraPosition -= left * cameraSpeed;
            if (pressedKeys.Contains(KeyQ)) cameraPosition -= up * cameraSpeed;
            if (pressedKeys.Contains(KeyE)) cameraPosition += up * cameraSpeed;
        }

        drawList.Reset();

        var titleSections = new[]
        {
            new TextArrangeSection
            {
                FontFile = font,
                NumChars = 15,
                FontIndex = 0,
                FontSize = 128.0f,
                Color = Vector4.One
            }
        };
        using (var arrangeResult = VgModule.ArrangeText(
            "Vector Graphics",
            titleSections,
            new RectF(0.0f, 0.0f, framebufferSize.Width, MathF.Max(framebufferSize.Height - 100.0f, 0.0f)),
            TextAlignment.Begin,
            TextAlignment.Center))
        {
            VgModule.CommitTextArrangeResult(arrangeResult, titleSections, fontAtlas, drawList);
        }

        shapeBuffer.SetShapePoints(Array.Empty<float>());
        drawList.SetShapeBuffer(shapeBuffer);

        const float shapeScale = 2.0f;
        var drawPosition = new Vector2(framebufferSize.Width / 2.0f - 350.0f * shapeScale, framebufferSize.Height - 500.0f * shapeScale);

        AppendShape(shapeBuffer, () => ShapeBuilder.AddRectangleFilled(shapeBuffer, 0.0f, 0.0f, 100.0f, 100.0f), drawList, drawPosition, VgPalette.LightPink);

        drawPosition.Y += 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddRectangleBordered(shapeBuffer, 0.0f, 0.0f, 100.0f, 100.0f, 5.0f, -2.5f), drawList, drawPosition, VgPalette.LightPink);

        drawPosition.X += 150.0f * shapeScale;
        drawPosition.Y -= 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddRoundedRectangleFilled(shapeBuffer, 0.0f, 0.0f, 100.0f, 100.0f, 10.0f), drawList, drawPosition, VgPalette.LightYellow);

        drawPosition.Y += 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddRoundedRectangleBordered(shapeBuffer, 0.0f, 0.0f, 100.0f, 100.0f, 10.0f, 5.0f, -2.5f), drawList, drawPosition, VgPalette.LightYellow);

        drawPosition.X += 150.0f * shapeScale;
        drawPosition.Y -= 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddTriangleFilled(shapeBuffer, 0.0f, 0.0f, 50.0f, 100.0f, 100.0f, 0.0f), drawList, drawPosition, VgPalette.LightGreen);

        drawPosition.Y += 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddTriangleBordered(shapeBuffer, 0.0f, 0.0f, 50.0f, 100.0f, 100.0f, 0.0f, 5.0f, -2.5f), drawList, drawPosition, VgPalette.LightGreen);

        drawPosition.X += 150.0f * shapeScale;
        drawPosition.Y -= 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddCircleFilled(shapeBuffer, 50.0f, 50.0f, 50.0f), drawList, drawPosition, VgPalette.LightBlue);

        drawPosition.Y += 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddCircleBordered(shapeBuffer, 50.0f, 50.0f, 50.0f, 5.0f, -2.5f), drawList, drawPosition, VgPalette.LightBlue);

        drawPosition.X += 150.0f * shapeScale;
        drawPosition.Y -= 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddAxisAlignedEllipseFilled(shapeBuffer, 50.0f, 50.0f, 50.0f, 25.0f), drawList, drawPosition, VgPalette.LightSteelBlue);

        drawPosition.Y += 150.0f * shapeScale;
        AppendShape(shapeBuffer, () => ShapeBuilder.AddAxisAlignedEllipseBordered(shapeBuffer, 50.0f, 50.0f, 50.0f, 25.0f, 5.0f, -2.5f), drawList, drawPosition, VgPalette.LightSteelBlue);

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
        var transformMatrix = CreateViewProjectionMatrix(framebufferSize, cameraPosition, yaw, pitch);

        shapeRenderer.Begin(backBuffer);
        shapeRenderer.Draw(vertexBuffer, indexBuffer, drawCalls, transformMatrix);
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

static void AppendShape(IShapeBuffer shapeBuffer, Action addShape, IShapeDrawList drawList, Vector2 drawPosition, Vector4 color)
{
    var start = (uint)shapeBuffer.GetShapePoints().Length;
    addShape();
    var end = (uint)shapeBuffer.GetShapePoints().Length;
    drawList.DrawShape(
        start,
        end - start,
        drawPosition,
        drawPosition + new Vector2(200.0f, 200.0f),
        Vector2.Zero,
        new Vector2(100.0f, 100.0f),
        color);
}

static (Vector3 Forward, Vector3 Left, Vector3 Up) ComputeCameraBasis(float yaw, float pitch)
{
    var forward = Vector3.Normalize(new Vector3(
        MathF.Sin(yaw) * MathF.Cos(pitch),
        -MathF.Sin(pitch),
        MathF.Cos(yaw) * MathF.Cos(pitch)));
    var left = Vector3.Normalize(Vector3.Cross(Vector3.UnitY, forward));
    if (!IsFinite(left))
    {
        left = Vector3.UnitX;
    }
    var up = Vector3.Normalize(Vector3.Cross(forward, left));
    return (forward, left, up);
}

static Matrix4x4 CreateViewProjectionMatrix(Size2U framebufferSize, Vector3 cameraPosition, float yaw, float pitch)
{
    var (forward, _, up) = ComputeCameraBasis(yaw, pitch);
    var target = cameraPosition + forward;
    var view = MakeLookAt(cameraPosition, target, up);
    var projection = MakePerspectiveFov(MathF.PI / 3.0f, framebufferSize.Width / (float)framebufferSize.Height, 0.3f, 10000.0f);
    return Multiply(view, projection);
}

static Matrix4x4 MakeLookAt(Vector3 eye, Vector3 target, Vector3 up)
{
    var rz = Vector3.Normalize(target - eye);
    var rx = Vector3.Normalize(Vector3.Cross(up, rz));
    var ry = Vector3.Cross(rz, rx);
    var negativeEye = -eye;
    var tx = Vector3.Dot(rx, negativeEye);
    var ty = Vector3.Dot(ry, negativeEye);
    var tz = Vector3.Dot(rz, negativeEye);
    return new Matrix4x4(
        rx.X, ry.X, rz.X, 0.0f,
        rx.Y, ry.Y, rz.Y, 0.0f,
        rx.Z, ry.Z, rz.Z, 0.0f,
        tx, ty, tz, 1.0f);
}

static Matrix4x4 MakePerspectiveFov(float fov, float aspect, float nearZ, float farZ)
{
    var halfFov = fov * 0.5f;
    var diagonal = MathF.Tan(halfFov);
    var height = diagonal / MathF.Sqrt(1.0f + aspect * aspect);
    var width = height * aspect;
    var range = farZ / (farZ - nearZ);
    return new Matrix4x4(
        1.0f / width, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / height, 0.0f, 0.0f,
        0.0f, 0.0f, range, 1.0f,
        0.0f, 0.0f, -range * nearZ, 0.0f);
}

static Matrix4x4 Multiply(Matrix4x4 left, Matrix4x4 right)
{
    return new Matrix4x4(
        left.M11 * right.M11 + left.M12 * right.M21 + left.M13 * right.M31 + left.M14 * right.M41,
        left.M11 * right.M12 + left.M12 * right.M22 + left.M13 * right.M32 + left.M14 * right.M42,
        left.M11 * right.M13 + left.M12 * right.M23 + left.M13 * right.M33 + left.M14 * right.M43,
        left.M11 * right.M14 + left.M12 * right.M24 + left.M13 * right.M34 + left.M14 * right.M44,
        left.M21 * right.M11 + left.M22 * right.M21 + left.M23 * right.M31 + left.M24 * right.M41,
        left.M21 * right.M12 + left.M22 * right.M22 + left.M23 * right.M32 + left.M24 * right.M42,
        left.M21 * right.M13 + left.M22 * right.M23 + left.M23 * right.M33 + left.M24 * right.M43,
        left.M21 * right.M14 + left.M22 * right.M24 + left.M23 * right.M34 + left.M24 * right.M44,
        left.M31 * right.M11 + left.M32 * right.M21 + left.M33 * right.M31 + left.M34 * right.M41,
        left.M31 * right.M12 + left.M32 * right.M22 + left.M33 * right.M32 + left.M34 * right.M42,
        left.M31 * right.M13 + left.M32 * right.M23 + left.M33 * right.M33 + left.M34 * right.M43,
        left.M31 * right.M14 + left.M32 * right.M24 + left.M33 * right.M34 + left.M34 * right.M44,
        left.M41 * right.M11 + left.M42 * right.M21 + left.M43 * right.M31 + left.M44 * right.M41,
        left.M41 * right.M12 + left.M42 * right.M22 + left.M43 * right.M32 + left.M44 * right.M42,
        left.M41 * right.M13 + left.M42 * right.M23 + left.M43 * right.M33 + left.M44 * right.M43,
        left.M41 * right.M14 + left.M42 * right.M24 + left.M43 * right.M34 + left.M44 * right.M44);
}

static float DegreesToRadians(float degrees) => degrees / 180.0f * MathF.PI;

static bool IsFinite(Vector3 value)
{
    return float.IsFinite(value.X) && float.IsFinite(value.Y) && float.IsFinite(value.Z);
}

static class VgPalette
{
    public static readonly Vector4 LightPink = new(1.0f, 0.7137255f, 0.7568628f, 1.0f);
    public static readonly Vector4 LightYellow = new(1.0f, 1.0f, 0.87843144f, 1.0f);
    public static readonly Vector4 LightGreen = new(0.5647059f, 0.9333334f, 0.5647059f, 1.0f);
    public static readonly Vector4 LightBlue = new(0.6784314f, 0.8470589f, 0.90196085f, 1.0f);
    public static readonly Vector4 LightSteelBlue = new(0.6901961f, 0.7686275f, 0.8705883f, 1.0f);
}
