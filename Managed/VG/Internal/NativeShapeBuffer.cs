using System;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.Runtime;

namespace Luna.VG.Internal;

internal sealed class NativeShapeBuffer : ObjectBase, IShapeBuffer
{
    private readonly IntPtr _shapeBuffer;

    internal NativeShapeBuffer(NativeShapeBufferHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IShapeBuffer == IntPtr.Zero)
        {
            throw new ArgumentException("Native shape buffer handle is incomplete.", nameof(handle));
        }
        _shapeBuffer = handle.IShapeBuffer;
    }

    public float[] GetShapePoints()
    {
        EnsureNotDisposed();
        var firstPassCode = new ErrorCode(VgNative.ShapeBufferGetPoints(_shapeBuffer, null, 0, out var count));
        var insufficientBuffer = RuntimeErrors.GetCodeByName("BasicError", "insufficient_user_buffer");
        if (firstPassCode.Failed && firstPassCode != insufficientBuffer)
        {
            RuntimeErrors.ThrowIfFailed(firstPassCode);
        }
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("The shape buffer contains too many points to copy into a managed array.");
        }
        var result = new float[(int)count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeBufferGetPoints(_shapeBuffer, result, count, out count)));
        return result;
    }

    public void SetShapePoints(float[] points)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(points);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeBufferSetPoints(_shapeBuffer, points, (ulong)points.Length)));
    }

    public IBuffer? Build(IDevice device)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(device);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeBufferBuild(_shapeBuffer, RhiDevice.GetNativeDevicePointer(device), out var buffer)));
        return buffer.Object == IntPtr.Zero ? null : new RhiBuffer(buffer.Object, buffer.IBuffer, retain: false);
    }

    internal static IntPtr GetNativeShapeBufferPointer(IShapeBuffer shapeBuffer)
    {
        ArgumentNullException.ThrowIfNull(shapeBuffer);
        if (shapeBuffer is not NativeShapeBuffer nativeShapeBuffer)
        {
            throw new ArgumentException("The shape buffer must be created by Luna.VG.", nameof(shapeBuffer));
        }
        nativeShapeBuffer.EnsureNotDisposed();
        return nativeShapeBuffer._shapeBuffer;
    }
}
