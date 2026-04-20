using System;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.RHIUtility.Internal;
using Luna.Runtime;

namespace Luna.RHIUtility;

internal sealed class BlitContext : RhiDeviceChild, IBlitContext
{
    private readonly IntPtr _iblitContext;

    internal BlitContext(IntPtr nativeObject, IntPtr nativeBlitContext, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeBlitContext == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeBlitContext));
        }
        _iblitContext = nativeBlitContext;
    }

    public void Reset()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.BlitContextReset(_iblitContext)));
    }

    public void Blit(
        ITexture destination,
        SubresourceIndex destinationSubresource,
        TextureViewDesc source,
        SamplerDesc sampler,
        BlitPoint topLeft,
        BlitPoint topRight,
        BlitPoint bottomLeft,
        BlitPoint bottomRight)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(destination);
        var nativeSource = NativeTextureViewDesc.FromPublic(source);
        var nativeSampler = new NativeSamplerDesc(sampler);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.BlitContextBlit(
            _iblitContext,
            destination.GetNativeHandle(),
            new NativeSubresourceIndex(destinationSubresource),
            in nativeSource,
            in nativeSampler,
            topLeft.X,
            topLeft.Y,
            topRight.X,
            topRight.Y,
            bottomLeft.X,
            bottomLeft.Y,
            bottomRight.X,
            bottomRight.Y)));
    }

    public void Commit(ICommandBuffer commandBuffer, bool submitAndWait)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(commandBuffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.BlitContextCommit(
            _iblitContext,
            commandBuffer.GetNativeHandle(),
            submitAndWait ? 1 : 0)));
    }
}
