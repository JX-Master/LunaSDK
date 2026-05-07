using System;
using System.Numerics;
using Luna.RHI;
using Luna.RHI.Internal;
using Luna.Runtime;

namespace Luna.VG.Internal;

internal sealed class NativeShapeRenderer : ObjectBase, IShapeRenderer
{
    private readonly IntPtr _shapeRenderer;

    internal NativeShapeRenderer(NativeShapeRendererHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IShapeRenderer == IntPtr.Zero)
        {
            throw new ArgumentException("Native shape renderer handle is incomplete.", nameof(handle));
        }
        _shapeRenderer = handle.IShapeRenderer;
    }

    public void Begin(ITexture renderTarget)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(renderTarget);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeRendererBegin(_shapeRenderer, RhiTexture.GetNativeTexturePointer(renderTarget))));
    }

    public void Draw(IBuffer vertexBuffer, IBuffer indexBuffer, ShapeDrawCall[] drawCalls)
    {
        EnsureNotDisposed();
        DrawCore(vertexBuffer, indexBuffer, drawCalls, null);
    }

    public void Draw(IBuffer vertexBuffer, IBuffer indexBuffer, ShapeDrawCall[] drawCalls, Matrix4x4 transformMatrix)
    {
        EnsureNotDisposed();
        DrawCore(vertexBuffer, indexBuffer, drawCalls, transformMatrix);
    }

    public void End()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeRendererEnd(_shapeRenderer)));
    }

    public void Submit(ICommandBuffer commandBuffer)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(commandBuffer);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeRendererSubmit(_shapeRenderer, RhiCommandBuffer.GetNativeCommandBufferPointer(commandBuffer))));
    }

    private void DrawCore(IBuffer vertexBuffer, IBuffer indexBuffer, ShapeDrawCall[] drawCalls, Matrix4x4? transformMatrix)
    {
        ArgumentNullException.ThrowIfNull(vertexBuffer);
        ArgumentNullException.ThrowIfNull(indexBuffer);
        ArgumentNullException.ThrowIfNull(drawCalls);
        var nativeDrawCalls = new NativeShapeDrawCall[drawCalls.Length];
        for (var i = 0; i < drawCalls.Length; ++i)
        {
            nativeDrawCalls[i] = NativeShapeDrawCall.FromPublic(drawCalls[i]);
        }
        var nativeVertexBuffer = RhiBuffer.GetNativeBufferPointer(vertexBuffer);
        var nativeIndexBuffer = RhiBuffer.GetNativeBufferPointer(indexBuffer);
        if (transformMatrix.HasValue)
        {
            var nativeTransform = NativeMatrix4x4.FromPublic(transformMatrix.Value);
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeRendererDrawWithTransform(
                _shapeRenderer,
                nativeVertexBuffer,
                nativeIndexBuffer,
                nativeDrawCalls,
                (ulong)nativeDrawCalls.Length,
                in nativeTransform)));
        }
        else
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VgNative.ShapeRendererDraw(
                _shapeRenderer,
                nativeVertexBuffer,
                nativeIndexBuffer,
                nativeDrawCalls,
                (ulong)nativeDrawCalls.Length)));
        }
    }

    internal static IntPtr GetNativeShapeRendererPointer(IShapeRenderer shapeRenderer)
    {
        ArgumentNullException.ThrowIfNull(shapeRenderer);
        if (shapeRenderer is not NativeShapeRenderer nativeShapeRenderer)
        {
            throw new ArgumentException("The shape renderer must be created by Luna.VG.", nameof(shapeRenderer));
        }
        nativeShapeRenderer.EnsureNotDisposed();
        return nativeShapeRenderer._shapeRenderer;
    }
}
