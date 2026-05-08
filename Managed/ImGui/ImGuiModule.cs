using System;
using Luna.ImGui.Internal;
using Luna.RHI;
using Luna.Runtime;
using Luna.Window;

namespace Luna.ImGui;

public static class ImGuiModule
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the ImGui module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.InitModule()));
    }

    public static void SetActiveWindow(IWindow? window)
    {
        ImGuiNative.SetActiveWindow(window?.GetNativeHandle() ?? IntPtr.Zero);
    }

    public static bool HandleWindowEvent(IObject eventObject)
    {
        ArgumentNullException.ThrowIfNull(eventObject);
        return ImGuiNative.HandleWindowEvent(eventObject.GetNativeHandle()) != 0;
    }

    public static void UpdateIo()
    {
        ImGuiNative.UpdateIo();
    }

    public static void AddDefaultFont(float fontSize = 18.0f)
    {
        ImGuiNative.AddDefaultFont(fontSize);
    }

    public static GlyphRange[] GetGlyphRangesDefault()
    {
        ImGuiNative.GetGlyphRangesDefault(null, 0, out var count);
        if (count == 0)
        {
            return Array.Empty<GlyphRange>();
        }
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("The glyph range table is too large to materialize in managed memory.");
        }
        var native = new NativeGlyphRange[(int)count];
        ImGuiNative.GetGlyphRangesDefault(native, count, out count);
        var result = new GlyphRange[(int)count];
        for (var i = 0; i < result.Length; ++i)
        {
            result[i] = native[i].ToPublic();
        }
        return result;
    }

    public static ISampledImage NewSampledImage(ITexture texture, SamplerDesc sampler)
    {
        ArgumentNullException.ThrowIfNull(texture);
        var nativeSampler = new NativeSamplerDesc(sampler);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.NewSampledImage(texture.GetNativeHandle(), in nativeSampler, out var image)));
        return new NativeSampledImage(image, retain: false);
    }

    public static void RenderDrawData(ICommandBuffer commandBuffer, ITexture renderTarget)
    {
        ArgumentNullException.ThrowIfNull(commandBuffer);
        ArgumentNullException.ThrowIfNull(renderTarget);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.RenderDrawData(commandBuffer.GetNativeHandle(), renderTarget.GetNativeHandle())));
    }
}
