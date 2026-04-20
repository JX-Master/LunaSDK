using System;
using Luna.RHI;
using Luna.RHIUtility.Internal;
using Luna.Runtime;

namespace Luna.RHIUtility;

internal sealed class MipmapGenerationContext : RhiDeviceChild, IMipmapGenerationContext
{
    private readonly IntPtr _imipmapGenerationContext;

    internal MipmapGenerationContext(IntPtr nativeObject, IntPtr nativeMipmapGenerationContext, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeMipmapGenerationContext == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeMipmapGenerationContext));
        }
        _imipmapGenerationContext = nativeMipmapGenerationContext;
    }

    public void Reset()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.MipmapGenerationContextReset(_imipmapGenerationContext)));
    }

    public void GenerateMipmaps(ITexture texture, uint sourceMip = 0, uint numGenerateMips = uint.MaxValue)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(texture);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.MipmapGenerationContextGenerateMipmaps(
            _imipmapGenerationContext,
            texture.GetNativeHandle(),
            sourceMip,
            numGenerateMips)));
    }

    public void Commit(ICommandBuffer commandBuffer, bool submitAndWait)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(commandBuffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.MipmapGenerationContextCommit(
            _imipmapGenerationContext,
            commandBuffer.GetNativeHandle(),
            submitAndWait ? 1 : 0)));
    }
}
