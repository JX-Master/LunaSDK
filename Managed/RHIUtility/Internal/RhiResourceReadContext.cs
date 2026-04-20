using System;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.RHIUtility.Internal;
using Luna.Runtime;

namespace Luna.RHIUtility;

internal sealed class ResourceReadContext : RhiDeviceChild, IResourceReadContext
{
    private readonly IntPtr _iresourceReadContext;

    internal ResourceReadContext(IntPtr nativeObject, IntPtr nativeResourceReadContext, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeResourceReadContext == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeResourceReadContext));
        }
        _iresourceReadContext = nativeResourceReadContext;
    }

    public void Reset()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceReadContextReset(_iresourceReadContext)));
    }

    public ulong ReadBuffer(IBuffer buffer, ulong offset, ulong size)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(buffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceReadContextReadBuffer(
            _iresourceReadContext,
            buffer.GetNativeHandle(),
            offset,
            size,
            out var handle)));
        return handle;
    }

    public ulong ReadTexture(
        ITexture texture,
        SubresourceIndex subresource,
        uint x,
        uint y,
        uint z,
        uint width,
        uint height,
        uint depth)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(texture);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceReadContextReadTexture(
            _iresourceReadContext,
            texture.GetNativeHandle(),
            new NativeSubresourceIndex(subresource),
            x,
            y,
            z,
            width,
            height,
            depth,
            out var handle)));
        return handle;
    }

    public void Commit(ICommandBuffer commandBuffer, bool submitAndWait)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(commandBuffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceReadContextCommit(
            _iresourceReadContext,
            commandBuffer.GetNativeHandle(),
            submitAndWait ? 1 : 0)));
    }

    public byte[] GetBufferData(ulong handle, ulong size)
    {
        EnsureNotDisposed();
        if (size > int.MaxValue)
        {
            throw new InvalidOperationException("The readback buffer data is too large to copy into a managed byte array.");
        }
        var data = new byte[(int)size];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceReadContextGetBufferData(
            _iresourceReadContext,
            handle,
            data.Length == 0 ? null : data,
            size)));
        return data;
    }

    public TextureReadData GetTextureData(ulong handle, uint copyBytesPerRow, uint height, uint depth)
    {
        EnsureNotDisposed();
        var size = checked((ulong)copyBytesPerRow * height * depth);
        if (size > int.MaxValue)
        {
            throw new InvalidOperationException("The readback texture data is too large to copy into a managed byte array.");
        }
        var data = new byte[(int)size];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiUtilityNative.ResourceReadContextGetTextureData(
            _iresourceReadContext,
            handle,
            copyBytesPerRow,
            height,
            depth,
            data.Length == 0 ? null : data,
            size,
            out var info)));
        return new TextureReadData(data, info.RowPitch, info.SlicePitch);
    }
}
