namespace Luna.RHI;

public interface ICommandBuffer : IDeviceChild
{
    uint CommandQueueIndex { get; }

    void Reset();

    void AttachDeviceObject(IDeviceChild deviceObject);

    void BeginEvent(string eventName);

    void EndEvent();

    void ResourceBarrier(TextureBarrier[] textureBarriers);

    void ResourceBarrier(BufferBarrier[] bufferBarriers, TextureBarrier[] textureBarriers);

    void BeginRenderPass(RenderPassDesc desc);

    void SetGraphicsPipelineLayout(IPipelineLayout pipelineLayout);

    void SetGraphicsPipelineState(IPipelineState pipelineState);

    void SetGraphicsDescriptorSet(uint index, IDescriptorSet descriptorSet);

    void SetGraphicsDescriptorSets(uint startIndex, IDescriptorSet[] descriptorSets);

    void SetVertexBuffer(uint slot, IBuffer buffer, ulong offset, uint size, uint elementSize);

    void SetVertexBuffers(uint startSlot, VertexBufferView[] views);

    void SetIndexBuffer(IBuffer buffer, ulong offset, uint size, Format format);

    void SetViewport(Viewport viewport);

    void SetViewports(Viewport[] viewports);

    void SetScissorRect(Luna.Window.RectI rect);

    void SetScissorRects(Luna.Window.RectI[] rects);

    void SetBlendFactor(Color4 blendFactor);

    void SetStencilRef(uint stencilRef);

    void Draw(uint vertexCount, uint startVertexLocation);

    void DrawIndexed(uint indexCount, uint startIndexLocation, int baseVertexLocation);

    void DrawInstanced(uint vertexCountPerInstance, uint instanceCount, uint startVertexLocation, uint startInstanceLocation);

    void DrawIndexedInstanced(uint indexCountPerInstance, uint instanceCount, uint startIndexLocation, int baseVertexLocation, uint startInstanceLocation);

    void BeginOcclusionQuery(OcclusionQueryMode mode, uint index);

    void EndOcclusionQuery(uint index);

    void EndRenderPass();

    void BeginComputePass(ComputePassDesc desc);

    void SetComputePipelineLayout(IPipelineLayout pipelineLayout);

    void SetComputePipelineState(IPipelineState pipelineState);

    void SetComputeDescriptorSet(uint index, IDescriptorSet descriptorSet);

    void SetComputeDescriptorSets(uint startIndex, IDescriptorSet[] descriptorSets);

    void Dispatch(uint threadGroupCountX, uint threadGroupCountY, uint threadGroupCountZ);

    void EndComputePass();

    void BeginCopyPass(CopyPassDesc desc);

    void CopyResource(IResource destination, IResource source);

    void CopyBuffer(IBuffer destination, ulong destinationOffset, IBuffer source, ulong sourceOffset, ulong copyBytes);

    void CopyTexture(
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
        uint copyDepth);

    void CopyBufferToTexture(
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
        uint copyDepth);

    void CopyTextureToBuffer(
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
        uint copyDepth);

    void EndCopyPass();

    void Submit(bool allowHostWaiting);

    void Submit(IFence[] waitFences, IFence[] signalFences, bool allowHostWaiting);

    void Wait();

    bool TryWait();
}
