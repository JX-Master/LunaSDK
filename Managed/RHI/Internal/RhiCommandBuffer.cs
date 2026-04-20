using System;
using Luna.Runtime;
using Luna.RHI.Internal;
using Luna.Window;

namespace Luna.RHI;

internal sealed class RhiCommandBuffer : RhiDeviceChild, ICommandBuffer
{
    private readonly IntPtr _icommandBuffer;

    internal RhiCommandBuffer(IntPtr nativeObject, IntPtr nativeCommandBuffer, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeCommandBuffer == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeCommandBuffer));
        }
        _icommandBuffer = nativeCommandBuffer;
    }

    public uint CommandQueueIndex
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferGetCommandQueueIndex(_icommandBuffer, out var index)));
            return index;
        }
    }

    public void Reset()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferReset(_icommandBuffer)));
    }

    public void AttachDeviceObject(IDeviceChild deviceObject)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(deviceObject);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferAttachDeviceObject(_icommandBuffer, deviceObject.GetNativeHandle())));
    }

    public void BeginEvent(string eventName)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(eventName);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferBeginEvent(_icommandBuffer, eventName)));
    }

    public void EndEvent()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferEndEvent(_icommandBuffer)));
    }

    public void ResourceBarrier(TextureBarrier[] textureBarriers)
    {
        ResourceBarrier(Array.Empty<BufferBarrier>(), textureBarriers);
    }

    public void ResourceBarrier(BufferBarrier[] bufferBarriers, TextureBarrier[] textureBarriers)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(bufferBarriers);
        ArgumentNullException.ThrowIfNull(textureBarriers);
        var nativeBufferBarriers = new NativeBufferBarrier[bufferBarriers.Length];
        for (var i = 0; i < bufferBarriers.Length; ++i)
        {
            nativeBufferBarriers[i] = NativeBufferBarrier.FromPublic(bufferBarriers[i]);
        }
        var nativeTextureBarriers = new NativeTextureBarrier[textureBarriers.Length];
        for (var i = 0; i < textureBarriers.Length; ++i)
        {
            nativeTextureBarriers[i] = NativeTextureBarrier.FromPublic(textureBarriers[i]);
        }

        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferResourceBarrier(
            _icommandBuffer,
            nativeBufferBarriers,
            (ulong)nativeBufferBarriers.Length,
            nativeTextureBarriers,
            (ulong)nativeTextureBarriers.Length)));
    }

    public void BeginRenderPass(RenderPassDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        if (desc.ColorAttachments.Length > 8)
        {
            throw new ArgumentOutOfRangeException(nameof(desc), "A render pass can use at most 8 color attachments.");
        }
        if (desc.ResolveAttachments.Length > 8)
        {
            throw new ArgumentOutOfRangeException(nameof(desc), "A render pass can use at most 8 resolve attachments.");
        }

        var nativeColorAttachments = new NativeColorAttachment[desc.ColorAttachments.Length];
        for (var i = 0; i < desc.ColorAttachments.Length; ++i)
        {
            nativeColorAttachments[i] = NativeColorAttachment.FromPublic(desc.ColorAttachments[i]);
        }
        var nativeResolveAttachments = new NativeResolveAttachment[desc.ResolveAttachments.Length];
        for (var i = 0; i < desc.ResolveAttachments.Length; ++i)
        {
            nativeResolveAttachments[i] = NativeResolveAttachment.FromPublic(desc.ResolveAttachments[i]);
        }

        if (desc.DepthStencilAttachment.HasValue)
        {
            var nativeDepthStencil = NativeDepthStencilAttachment.FromPublic(desc.DepthStencilAttachment.Value);
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferBeginRenderPassWithDepth(
                _icommandBuffer,
                nativeColorAttachments,
                (uint)nativeColorAttachments.Length,
                nativeResolveAttachments,
                (uint)nativeResolveAttachments.Length,
                in nativeDepthStencil,
                GetNativeQueryHeapPointerOrZero(desc.OcclusionQueryHeap),
                GetNativeQueryHeapPointerOrZero(desc.TimestampQueryHeap),
                GetNativeQueryHeapPointerOrZero(desc.PipelineStatisticsQueryHeap),
                desc.TimestampQueryBeginPassWriteIndex,
                desc.TimestampQueryEndPassWriteIndex,
                desc.PipelineStatisticsQueryWriteIndex,
                desc.ArraySize,
                desc.SampleCount)));
        }
        else
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferBeginRenderPassNoDepth(
                _icommandBuffer,
                nativeColorAttachments,
                (uint)nativeColorAttachments.Length,
                nativeResolveAttachments,
                (uint)nativeResolveAttachments.Length,
                IntPtr.Zero,
                GetNativeQueryHeapPointerOrZero(desc.OcclusionQueryHeap),
                GetNativeQueryHeapPointerOrZero(desc.TimestampQueryHeap),
                GetNativeQueryHeapPointerOrZero(desc.PipelineStatisticsQueryHeap),
                desc.TimestampQueryBeginPassWriteIndex,
                desc.TimestampQueryEndPassWriteIndex,
                desc.PipelineStatisticsQueryWriteIndex,
                desc.ArraySize,
                desc.SampleCount)));
        }
    }

    public void EndRenderPass()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferEndRenderPass(_icommandBuffer)));
    }

    public void SetGraphicsPipelineLayout(IPipelineLayout pipelineLayout)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetGraphicsPipelineLayout(
            _icommandBuffer,
            RhiPipelineLayout.GetNativePipelineLayoutPointer(pipelineLayout))));
    }

    public void SetGraphicsPipelineState(IPipelineState pipelineState)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetGraphicsPipelineState(
            _icommandBuffer,
            RhiPipelineState.GetNativePipelineStatePointer(pipelineState))));
    }

    public void SetGraphicsDescriptorSet(uint index, IDescriptorSet descriptorSet)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetGraphicsDescriptorSet(
            _icommandBuffer,
            index,
            RhiDescriptorSet.GetNativeDescriptorSetPointer(descriptorSet))));
    }

    public void SetGraphicsDescriptorSets(uint startIndex, IDescriptorSet[] descriptorSets)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(descriptorSets);
        var nativeDescriptorSets = ToNativeDescriptorSetPointers(descriptorSets);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetGraphicsDescriptorSets(
            _icommandBuffer,
            startIndex,
            nativeDescriptorSets.Length == 0 ? null : nativeDescriptorSets,
            (ulong)nativeDescriptorSets.Length)));
    }

    public void SetVertexBuffer(uint slot, IBuffer buffer, ulong offset, uint size, uint elementSize)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetVertexBuffer(
            _icommandBuffer,
            slot,
            RhiBuffer.GetNativeBufferPointer(buffer),
            offset,
            size,
            elementSize)));
    }

    public void SetVertexBuffers(uint startSlot, VertexBufferView[] views)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(views);
        var nativeViews = new NativeVertexBufferView[views.Length];
        for (var i = 0; i < views.Length; ++i)
        {
            nativeViews[i] = NativeVertexBufferView.FromPublic(views[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetVertexBuffers(
            _icommandBuffer,
            startSlot,
            nativeViews,
            (ulong)nativeViews.Length)));
    }

    public void SetIndexBuffer(IBuffer buffer, ulong offset, uint size, Format format)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetIndexBuffer(
            _icommandBuffer,
            RhiBuffer.GetNativeBufferPointer(buffer),
            offset,
            size,
            (uint)format)));
    }

    public void SetViewport(Viewport viewport)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetViewport(
            _icommandBuffer,
            viewport.TopLeftX,
            viewport.TopLeftY,
            viewport.Width,
            viewport.Height,
            viewport.MinDepth,
            viewport.MaxDepth)));
    }

    public void SetViewports(Viewport[] viewports)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(viewports);
        var nativeViewports = new NativeViewport[viewports.Length];
        for (var i = 0; i < nativeViewports.Length; ++i)
        {
            nativeViewports[i] = new NativeViewport(viewports[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetViewports(
            _icommandBuffer,
            nativeViewports.Length == 0 ? null : nativeViewports,
            (ulong)nativeViewports.Length)));
    }

    public void SetScissorRect(RectI rect)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetScissorRect(
            _icommandBuffer,
            rect.OffsetX,
            rect.OffsetY,
            rect.Width,
            rect.Height)));
    }

    public void SetScissorRects(RectI[] rects)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(rects);
        var nativeRects = new NativeRhiRectI[rects.Length];
        for (var i = 0; i < nativeRects.Length; ++i)
        {
            nativeRects[i] = new NativeRhiRectI(rects[i]);
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetScissorRects(
            _icommandBuffer,
            nativeRects.Length == 0 ? null : nativeRects,
            (ulong)nativeRects.Length)));
    }

    public void SetBlendFactor(Color4 blendFactor)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetBlendFactor(
            _icommandBuffer,
            blendFactor.Red,
            blendFactor.Green,
            blendFactor.Blue,
            blendFactor.Alpha)));
    }

    public void SetStencilRef(uint stencilRef)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetStencilRef(_icommandBuffer, stencilRef)));
    }

    public void Draw(uint vertexCount, uint startVertexLocation)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferDraw(_icommandBuffer, vertexCount, startVertexLocation)));
    }

    public void DrawIndexed(uint indexCount, uint startIndexLocation, int baseVertexLocation)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferDrawIndexed(_icommandBuffer, indexCount, startIndexLocation, baseVertexLocation)));
    }

    public void DrawInstanced(uint vertexCountPerInstance, uint instanceCount, uint startVertexLocation, uint startInstanceLocation)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferDrawInstanced(
            _icommandBuffer,
            vertexCountPerInstance,
            instanceCount,
            startVertexLocation,
            startInstanceLocation)));
    }

    public void DrawIndexedInstanced(uint indexCountPerInstance, uint instanceCount, uint startIndexLocation, int baseVertexLocation, uint startInstanceLocation)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferDrawIndexedInstanced(
            _icommandBuffer,
            indexCountPerInstance,
            instanceCount,
            startIndexLocation,
            baseVertexLocation,
            startInstanceLocation)));
    }

    public void BeginOcclusionQuery(OcclusionQueryMode mode, uint index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferBeginOcclusionQuery(_icommandBuffer, (uint)mode, index)));
    }

    public void EndOcclusionQuery(uint index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferEndOcclusionQuery(_icommandBuffer, index)));
    }

    public void BeginComputePass(ComputePassDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        var nativeDesc = new NativeComputePassDesc(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferBeginComputePass(_icommandBuffer, in nativeDesc)));
    }

    public void SetComputePipelineLayout(IPipelineLayout pipelineLayout)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetComputePipelineLayout(
            _icommandBuffer,
            RhiPipelineLayout.GetNativePipelineLayoutPointer(pipelineLayout))));
    }

    public void SetComputePipelineState(IPipelineState pipelineState)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetComputePipelineState(
            _icommandBuffer,
            RhiPipelineState.GetNativePipelineStatePointer(pipelineState))));
    }

    public void SetComputeDescriptorSet(uint index, IDescriptorSet descriptorSet)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetComputeDescriptorSet(
            _icommandBuffer,
            index,
            RhiDescriptorSet.GetNativeDescriptorSetPointer(descriptorSet))));
    }

    public void SetComputeDescriptorSets(uint startIndex, IDescriptorSet[] descriptorSets)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(descriptorSets);
        var nativeDescriptorSets = ToNativeDescriptorSetPointers(descriptorSets);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSetComputeDescriptorSets(
            _icommandBuffer,
            startIndex,
            nativeDescriptorSets.Length == 0 ? null : nativeDescriptorSets,
            (ulong)nativeDescriptorSets.Length)));
    }

    public void Dispatch(uint threadGroupCountX, uint threadGroupCountY, uint threadGroupCountZ)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferDispatch(_icommandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ)));
    }

    public void EndComputePass()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferEndComputePass(_icommandBuffer)));
    }

    public void BeginCopyPass(CopyPassDesc desc)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(desc);
        var nativeDesc = new NativeCopyPassDesc(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferBeginCopyPass(_icommandBuffer, in nativeDesc)));
    }

    public void CopyResource(IResource destination, IResource source)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(destination);
        ArgumentNullException.ThrowIfNull(source);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferCopyResource(
            _icommandBuffer,
            destination.GetNativeHandle(),
            source.GetNativeHandle())));
    }

    public void CopyBuffer(IBuffer destination, ulong destinationOffset, IBuffer source, ulong sourceOffset, ulong copyBytes)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferCopyBuffer(
            _icommandBuffer,
            RhiBuffer.GetNativeBufferPointer(destination),
            destinationOffset,
            RhiBuffer.GetNativeBufferPointer(source),
            sourceOffset,
            copyBytes)));
    }

    public void CopyTexture(
        ITexture destination,
        SubresourceIndex destinationSubresource,
        uint destinationX,
        uint destinationY,
        uint destinationZ,
        ITexture source,
        SubresourceIndex sourceSubresource,
        uint sourceX,
        uint sourceY,
        uint sourceZ,
        uint copyWidth,
        uint copyHeight,
        uint copyDepth)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferCopyTexture(
            _icommandBuffer,
            RhiTexture.GetNativeTexturePointer(destination),
            new NativeSubresourceIndex(destinationSubresource),
            destinationX,
            destinationY,
            destinationZ,
            RhiTexture.GetNativeTexturePointer(source),
            new NativeSubresourceIndex(sourceSubresource),
            sourceX,
            sourceY,
            sourceZ,
            copyWidth,
            copyHeight,
            copyDepth)));
    }

    public void CopyBufferToTexture(
        ITexture destination,
        SubresourceIndex destinationSubresource,
        uint destinationX,
        uint destinationY,
        uint destinationZ,
        IBuffer source,
        ulong sourceOffset,
        uint sourceRowPitch,
        uint sourceSlicePitch,
        uint copyWidth,
        uint copyHeight,
        uint copyDepth)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferCopyBufferToTexture(
            _icommandBuffer,
            RhiTexture.GetNativeTexturePointer(destination),
            new NativeSubresourceIndex(destinationSubresource),
            destinationX,
            destinationY,
            destinationZ,
            RhiBuffer.GetNativeBufferPointer(source),
            sourceOffset,
            sourceRowPitch,
            sourceSlicePitch,
            copyWidth,
            copyHeight,
            copyDepth)));
    }

    public void CopyTextureToBuffer(
        IBuffer destination,
        ulong destinationOffset,
        uint destinationRowPitch,
        uint destinationSlicePitch,
        ITexture source,
        SubresourceIndex sourceSubresource,
        uint sourceX,
        uint sourceY,
        uint sourceZ,
        uint copyWidth,
        uint copyHeight,
        uint copyDepth)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferCopyTextureToBuffer(
            _icommandBuffer,
            RhiBuffer.GetNativeBufferPointer(destination),
            destinationOffset,
            destinationRowPitch,
            destinationSlicePitch,
            RhiTexture.GetNativeTexturePointer(source),
            new NativeSubresourceIndex(sourceSubresource),
            sourceX,
            sourceY,
            sourceZ,
            copyWidth,
            copyHeight,
            copyDepth)));
    }

    public void EndCopyPass()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferEndCopyPass(_icommandBuffer)));
    }

    public void Submit(bool allowHostWaiting)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSubmit(_icommandBuffer, allowHostWaiting ? 1 : 0)));
    }

    public void Submit(IFence[] waitFences, IFence[] signalFences, bool allowHostWaiting)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(waitFences);
        ArgumentNullException.ThrowIfNull(signalFences);
        var nativeWaitFences = ToNativeFencePointers(waitFences);
        var nativeSignalFences = ToNativeFencePointers(signalFences);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferSubmitWithFences(
            _icommandBuffer,
            nativeWaitFences.Length == 0 ? null : nativeWaitFences,
            (ulong)nativeWaitFences.Length,
            nativeSignalFences.Length == 0 ? null : nativeSignalFences,
            (ulong)nativeSignalFences.Length,
            allowHostWaiting ? 1 : 0)));
    }

    public void Wait()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferWait(_icommandBuffer)));
    }

    public bool TryWait()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.CommandBufferTryWait(_icommandBuffer, out var waited)));
        return waited != 0;
    }

    private static IntPtr[] ToNativeFencePointers(IFence[] fences)
    {
        var nativeFences = new IntPtr[fences.Length];
        for (var i = 0; i < nativeFences.Length; ++i)
        {
            nativeFences[i] = RhiFence.GetNativeFencePointer(fences[i]);
        }
        return nativeFences;
    }

    private static IntPtr[] ToNativeDescriptorSetPointers(IDescriptorSet[] descriptorSets)
    {
        var nativeDescriptorSets = new IntPtr[descriptorSets.Length];
        for (var i = 0; i < nativeDescriptorSets.Length; ++i)
        {
            nativeDescriptorSets[i] = RhiDescriptorSet.GetNativeDescriptorSetPointer(descriptorSets[i]);
        }
        return nativeDescriptorSets;
    }

    private static IntPtr GetNativeQueryHeapPointerOrZero(IQueryHeap? queryHeap)
    {
        return queryHeap is null ? IntPtr.Zero : RhiQueryHeap.GetNativeQueryHeapPointer(queryHeap);
    }
}
