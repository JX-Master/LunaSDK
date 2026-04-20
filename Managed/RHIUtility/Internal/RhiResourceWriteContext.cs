using System;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.RHIUtility.Internal;
using Luna.Runtime;

namespace Luna.RHIUtility;

internal sealed class ResourceWriteContext : RhiDeviceChild, IResourceWriteContext
{
    private readonly IntPtr _iresourceWriteContext;

    internal ResourceWriteContext(IntPtr nativeObject, IntPtr nativeResourceWriteContext, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeResourceWriteContext == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeResourceWriteContext));
        }
        _iresourceWriteContext = nativeResourceWriteContext;
    }

    public void Reset()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceWriteContextReset(_iresourceWriteContext)));
    }

    public void WriteBuffer(IBuffer buffer, ulong offset, byte[] data)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(buffer);
        ArgumentNullException.ThrowIfNull(data);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceWriteContextWriteBuffer(
            _iresourceWriteContext,
            buffer.GetNativeHandle(),
            offset,
            data.Length == 0 ? null : data,
            (ulong)data.Length)));
    }

    public TextureWriteInfo WriteTexture(
        ITexture texture,
        SubresourceIndex subresource,
        uint x,
        uint y,
        uint z,
        uint width,
        uint height,
        uint depth,
        byte[] data,
        uint sourceRowPitch,
        uint sourceSlicePitch,
        uint copyBytesPerRow)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(texture);
        ArgumentNullException.ThrowIfNull(data);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceWriteContextWriteTexture(
            _iresourceWriteContext,
            texture.GetNativeHandle(),
            new NativeSubresourceIndex(subresource),
            x,
            y,
            z,
            width,
            height,
            depth,
            data,
            (ulong)data.Length,
            sourceRowPitch,
            sourceSlicePitch,
            copyBytesPerRow,
            out var info)));
        return info.ToPublic();
    }

    public void Commit(ICommandBuffer commandBuffer, bool submitAndWait)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(commandBuffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceWriteContextCommit(
            _iresourceWriteContext,
            commandBuffer.GetNativeHandle(),
            submitAndWait ? 1 : 0)));
    }
}
