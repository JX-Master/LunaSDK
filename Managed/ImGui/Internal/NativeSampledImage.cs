using System;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.Runtime;

namespace Luna.ImGui.Internal;

internal sealed class NativeSampledImage : ObjectBase, ISampledImage
{
    private readonly IntPtr _isampledImage;

    internal NativeSampledImage(NativeSampledImageHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.ISampledImage == IntPtr.Zero)
        {
            throw new ArgumentException("Native sampled image handle is incomplete.", nameof(handle));
        }
        _isampledImage = handle.ISampledImage;
    }

    public ITexture? Texture
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.SampledImageGetTexture(_isampledImage, out var texture)));
            return texture.Object == IntPtr.Zero ? null : new RhiTexture(texture.Object, texture.ITexture, retain: false);
        }
        set
        {
            EnsureNotDisposed();
            ImGuiNative.SampledImageSetTexture(_isampledImage, value?.GetNativeHandle() ?? IntPtr.Zero);
        }
    }

    public SamplerDesc Sampler
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(ImGuiNative.SampledImageGetSampler(_isampledImage, out var sampler)));
            return sampler.ToPublic();
        }
        set
        {
            EnsureNotDisposed();
            var native = new NativeSamplerDesc(value);
            ImGuiNative.SampledImageSetSampler(_isampledImage, in native);
        }
    }

    internal static IntPtr GetNativeSampledImagePointer(ISampledImage sampledImage)
    {
        ArgumentNullException.ThrowIfNull(sampledImage);
        if (sampledImage is not NativeSampledImage nativeSampledImage)
        {
            throw new ArgumentException("The sampled image must be created by Luna.ImGui.", nameof(sampledImage));
        }
        nativeSampledImage.EnsureNotDisposed();
        return nativeSampledImage._isampledImage;
    }
}
