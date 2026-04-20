using System;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal sealed class RhiBuffer : RhiResource, IBuffer
{
    private readonly IntPtr _ibuffer;

    internal RhiBuffer(IntPtr nativeObject, IntPtr nativeBuffer, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeBuffer == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeBuffer));
        }
        _ibuffer = nativeBuffer;
    }

    public BufferDesc Desc
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.BufferGetDesc(_ibuffer, out var desc)));
            return desc.ToPublic();
        }
    }

    public void Write(ulong offset, byte[] data)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(data);
        if (data.Length == 0)
        {
            return;
        }

        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.BufferMap(_ibuffer, 0, 0, out var mappedData)));
        try
        {
            Marshal.Copy(data, 0, IntPtr.Add(mappedData, checked((int)offset)), data.Length);
        }
        finally
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.BufferUnmap(_ibuffer, offset, offset + (ulong)data.Length)));
        }
    }

    internal static IntPtr GetNativeBufferPointer(IBuffer buffer)
    {
        ArgumentNullException.ThrowIfNull(buffer);
        if (buffer is not RhiBuffer nativeBuffer)
        {
            throw new ArgumentException("The buffer must be created by Luna.RHI.", nameof(buffer));
        }
        nativeBuffer.EnsureNotDisposed();
        return nativeBuffer._ibuffer;
    }
}
