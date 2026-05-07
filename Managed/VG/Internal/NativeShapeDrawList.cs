using System;
using System.Numerics;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.Runtime;

namespace Luna.VG.Internal;

internal sealed class NativeShapeDrawList : ObjectBase, IShapeDrawList
{
    private readonly IntPtr _shapeDrawList;

    internal NativeShapeDrawList(NativeShapeDrawListHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IShapeDrawList == IntPtr.Zero)
        {
            throw new ArgumentException("Native shape draw list handle is incomplete.", nameof(handle));
        }
        _shapeDrawList = handle.IShapeDrawList;
    }

    public IDevice GetDevice()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetDevice(_shapeDrawList, out var device)));
        return new RhiDevice(device.Object, device.IDevice, retain: false);
    }

    public void Reset()
    {
        EnsureNotDisposed();
        VgNative.ShapeDrawListReset(_shapeDrawList);
    }

    public void SetShapeBuffer(IShapeBuffer? shapeBuffer)
    {
        EnsureNotDisposed();
        VgNative.ShapeDrawListSetShapeBuffer(_shapeDrawList, shapeBuffer is null ? IntPtr.Zero : NativeShapeBuffer.GetNativeShapeBufferPointer(shapeBuffer));
    }

    public IShapeBuffer GetShapeBuffer()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetShapeBuffer(_shapeDrawList, out var shapeBuffer)));
        return new NativeShapeBuffer(shapeBuffer, retain: false);
    }

    public void SetTexture(ITexture? texture)
    {
        EnsureNotDisposed();
        VgNative.ShapeDrawListSetTexture(_shapeDrawList, texture is null ? IntPtr.Zero : RhiTexture.GetNativeTexturePointer(texture));
    }

    public ITexture? GetTexture()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetTexture(_shapeDrawList, out var texture)));
        return texture.Object == IntPtr.Zero ? null : new RhiTexture(texture.Object, texture.ITexture, retain: false);
    }

    public void SetSampler(SamplerDesc? desc)
    {
        EnsureNotDisposed();
        if (desc.HasValue)
        {
            var sampler = new NativeSamplerDesc(desc.Value);
            VgNative.ShapeDrawListSetSampler(_shapeDrawList, in sampler);
        }
        else
        {
            VgNative.ShapeDrawListResetSampler(_shapeDrawList);
        }
    }

    public SamplerDesc GetSampler()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetSampler(_shapeDrawList, out var sampler)));
        return sampler.ToPublic();
    }

    public void SetTransform(Matrix4x4 transform)
    {
        EnsureNotDisposed();
        var nativeTransform = NativeMatrix4x4.FromPublic(transform);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListSetTransform(_shapeDrawList, in nativeTransform)));
    }

    public Matrix4x4 GetTransform()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetTransform(_shapeDrawList, out var transform)));
        return transform.ToPublic();
    }

    public void SetClipRect(RectF clipRect)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListSetClipRect(_shapeDrawList, in clipRect)));
    }

    public RectF GetClipRect()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetClipRect(_shapeDrawList, out var clipRect)));
        return clipRect;
    }

    public void DrawShapeRaw(Vertex[] vertices, uint[] indices)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(vertices);
        ArgumentNullException.ThrowIfNull(indices);
        var nativeVertices = new NativeVertex[vertices.Length];
        for (var i = 0; i < vertices.Length; ++i)
        {
            nativeVertices[i] = NativeVertex.FromPublic(vertices[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListDrawShapeRaw(_shapeDrawList, nativeVertices, (ulong)nativeVertices.Length, indices, (ulong)indices.Length)));
    }

    public void DrawShape(uint beginCommand, uint numCommands, Vector2 minPosition, Vector2 maxPosition, Vector2 minShapeCoord, Vector2 maxShapeCoord, Vector4? color = null, Vector2? minTexCoord = null, Vector2? maxTexCoord = null)
    {
        EnsureNotDisposed();
        var nativeColor = NativeFloat4.FromPublic(color ?? Vector4.One);
        var nativeMinTexCoord = NativeFloat2.FromPublic(minTexCoord ?? Vector2.Zero);
        var nativeMaxTexCoord = NativeFloat2.FromPublic(maxTexCoord ?? Vector2.Zero);
        var nativeMinPosition = NativeFloat2.FromPublic(minPosition);
        var nativeMaxPosition = NativeFloat2.FromPublic(maxPosition);
        var nativeMinShapeCoord = NativeFloat2.FromPublic(minShapeCoord);
        var nativeMaxShapeCoord = NativeFloat2.FromPublic(maxShapeCoord);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListDrawShape(
            _shapeDrawList,
            beginCommand,
            numCommands,
            in nativeMinPosition,
            in nativeMaxPosition,
            in nativeMinShapeCoord,
            in nativeMaxShapeCoord,
            in nativeColor,
            in nativeMinTexCoord,
            in nativeMaxTexCoord)));
    }

    public void Compile()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListCompile(_shapeDrawList)));
    }

    public IBuffer? GetVertexBuffer()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetVertexBuffer(_shapeDrawList, out var buffer)));
        return buffer.Object == IntPtr.Zero ? null : new RhiBuffer(buffer.Object, buffer.IBuffer, retain: false);
    }

    public uint GetVertexBufferSize()
    {
        EnsureNotDisposed();
        return VgNative.ShapeDrawListGetVertexBufferSize(_shapeDrawList);
    }

    public IBuffer? GetIndexBuffer()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetIndexBuffer(_shapeDrawList, out var buffer)));
        return buffer.Object == IntPtr.Zero ? null : new RhiBuffer(buffer.Object, buffer.IBuffer, retain: false);
    }

    public uint GetIndexBufferSize()
    {
        EnsureNotDisposed();
        return VgNative.ShapeDrawListGetIndexBufferSize(_shapeDrawList);
    }

    public ShapeDrawCall[] GetDrawCalls()
    {
        EnsureNotDisposed();
        var count = VgNative.ShapeDrawListGetDrawCallCount(_shapeDrawList);
        if (count > int.MaxValue)
        {
            throw new InvalidOperationException("The shape draw list has too many draw calls to copy into a managed array.");
        }
        var result = new ShapeDrawCall[(int)count];
        for (ulong i = 0; i < count; ++i)
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeDrawListGetDrawCall(_shapeDrawList, i, out var drawCall)));
            var shapeBuffer = drawCall.ShapeBuffer.Object == IntPtr.Zero ? null : new RhiBuffer(drawCall.ShapeBuffer.Object, drawCall.ShapeBuffer.IBuffer, retain: false);
            var texture = drawCall.Texture.Object == IntPtr.Zero ? null : new RhiTexture(drawCall.Texture.Object, drawCall.Texture.ITexture, retain: false);
            result[(int)i] = new ShapeDrawCall(shapeBuffer, texture, drawCall.Sampler.ToPublic(), drawCall.ClipRect, drawCall.BaseIndex, drawCall.NumIndices, drawCall.Transform.ToPublic());
        }
        return result;
    }

    internal static IntPtr GetNativeShapeDrawListPointer(IShapeDrawList drawList)
    {
        ArgumentNullException.ThrowIfNull(drawList);
        if (drawList is not NativeShapeDrawList nativeShapeDrawList)
        {
            throw new ArgumentException("The shape draw list must be created by Luna.VG.", nameof(drawList));
        }
        nativeShapeDrawList.EnsureNotDisposed();
        return nativeShapeDrawList._shapeDrawList;
    }
}
