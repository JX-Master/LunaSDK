using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

internal static class RhiNative
{
    private const string LibraryName = "LunaRHIC";

    [DllImport(LibraryName, EntryPoint = "luna_rhi_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_rhi_get_backend_type")]
    internal static extern uint GetBackendType();

    [DllImport(LibraryName, EntryPoint = "luna_rhi_get_main_device")]
    internal static extern UIntPtr GetMainDevice(out NativeRhiDeviceHandle outDevice);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_new_device")]
    internal static extern UIntPtr NewDevice(IntPtr adapter, out NativeRhiDeviceHandle outDevice);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_get_num_adapters")]
    internal static extern UIntPtr GetNumAdapters(out uint outCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_get_adapter")]
    internal static extern UIntPtr GetAdapter(uint index, out NativeRhiAdapterHandle outAdapter);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_adapter_get_name")]
    internal static extern UIntPtr AdapterGetName(IntPtr adapter, out IntPtr outName);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_child_get_device")]
    internal static extern UIntPtr DeviceChildGetDevice(IntPtr deviceChildObject, out NativeRhiDeviceHandle outDevice);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_child_set_name")]
    internal static extern UIntPtr DeviceChildSetName(
        IntPtr deviceChildObject,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_get_num_command_queues")]
    internal static extern UIntPtr DeviceGetNumCommandQueues(IntPtr device, out uint outCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_get_command_queue_desc")]
    internal static extern UIntPtr DeviceGetCommandQueueDesc(IntPtr device, uint index, out NativeCommandQueueDesc outDesc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_get_command_queue_timestamp_frequency")]
    internal static extern UIntPtr DeviceGetCommandQueueTimestampFrequency(IntPtr device, uint index, out double outFrequency);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_check_feature")]
    internal static extern UIntPtr DeviceCheckFeature(IntPtr device, uint feature, out ulong outValue);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_get_texture_data_placement_info")]
    internal static extern UIntPtr DeviceGetTextureDataPlacementInfo(
        IntPtr device,
        uint width,
        uint height,
        uint depth,
        uint format,
        out NativeTextureDataPlacementInfo outInfo);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_is_resources_aliasing_compatible")]
    internal static extern UIntPtr DeviceIsResourcesAliasingCompatible(
        IntPtr device,
        uint memoryType,
        [In] NativeBufferDesc[]? buffers,
        ulong bufferCount,
        [In] NativeTextureDesc[]? textures,
        ulong textureCount,
        out int outCompatible);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_allocate_memory")]
    internal static extern UIntPtr DeviceAllocateMemory(
        IntPtr device,
        uint memoryType,
        [In] NativeBufferDesc[]? buffers,
        ulong bufferCount,
        [In] NativeTextureDesc[]? textures,
        ulong textureCount,
        out NativeRhiDeviceMemoryHandle outMemory);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_buffer")]
    internal static extern UIntPtr DeviceNewBuffer(IntPtr device, uint memoryType, in NativeBufferDesc desc, out NativeRhiBufferHandle outBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_texture")]
    internal static extern UIntPtr DeviceNewTexture(IntPtr device, uint memoryType, in NativeTextureDesc desc, out NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_texture_with_clear_value")]
    internal static extern UIntPtr DeviceNewTextureWithClearValue(IntPtr device, uint memoryType, in NativeTextureDesc desc, in NativeClearValue optimizedClearValue, out NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_aliasing_buffer")]
    internal static extern UIntPtr DeviceNewAliasingBuffer(IntPtr device, IntPtr deviceMemory, in NativeBufferDesc desc, out NativeRhiBufferHandle outBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_aliasing_texture")]
    internal static extern UIntPtr DeviceNewAliasingTexture(IntPtr device, IntPtr deviceMemory, in NativeTextureDesc desc, out NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_aliasing_texture_with_clear_value")]
    internal static extern UIntPtr DeviceNewAliasingTextureWithClearValue(IntPtr device, IntPtr deviceMemory, in NativeTextureDesc desc, in NativeClearValue optimizedClearValue, out NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_fence")]
    internal static extern UIntPtr DeviceNewFence(IntPtr device, out NativeRhiFenceHandle outFence);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_query_heap")]
    internal static extern UIntPtr DeviceNewQueryHeap(IntPtr device, in NativeQueryHeapDesc desc, out NativeRhiQueryHeapHandle outQueryHeap);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_descriptor_set_layout")]
    internal static extern UIntPtr DeviceNewDescriptorSetLayout(
        IntPtr device,
        [In] NativeDescriptorSetLayoutBinding[]? bindings,
        ulong bindingCount,
        uint flags,
        out NativeRhiDescriptorSetLayoutHandle outDescriptorSetLayout);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_descriptor_set")]
    internal static extern UIntPtr DeviceNewDescriptorSet(
        IntPtr device,
        IntPtr descriptorSetLayout,
        uint numVariableDescriptors,
        out NativeRhiDescriptorSetHandle outDescriptorSet);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_pipeline_layout")]
    internal static extern UIntPtr DeviceNewPipelineLayout(
        IntPtr device,
        [In] IntPtr[]? descriptorSetLayouts,
        ulong descriptorSetLayoutCount,
        uint flags,
        out NativeRhiPipelineLayoutHandle outPipelineLayout);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_graphics_pipeline_state")]
    internal static extern UIntPtr DeviceNewGraphicsPipelineState(
        IntPtr device,
        NativeGraphicsPipelineStateDesc desc,
        out NativeRhiPipelineStateHandle outPipelineState);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_compute_pipeline_state")]
    internal static extern UIntPtr DeviceNewComputePipelineState(
        IntPtr device,
        NativeComputePipelineStateDesc desc,
        out NativeRhiPipelineStateHandle outPipelineState);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_swap_chain")]
    internal static extern UIntPtr DeviceNewSwapChain(
        IntPtr device,
        uint commandQueueIndex,
        IntPtr windowObject,
        in NativeSwapChainDesc desc,
        out NativeRhiSwapChainHandle outSwapChain);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_new_command_buffer")]
    internal static extern UIntPtr DeviceNewCommandBuffer(IntPtr device, uint commandQueueIndex, out NativeRhiCommandBufferHandle outCommandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_get_desc")]
    internal static extern UIntPtr SwapChainGetDesc(IntPtr swapChain, out NativeSwapChainDesc outDesc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_get_window")]
    internal static extern UIntPtr SwapChainGetWindow(IntPtr swapChain, out NativeRhiWindowHandle outWindow);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_get_surface_transform")]
    internal static extern UIntPtr SwapChainGetSurfaceTransform(IntPtr swapChain, out uint outTransform);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_get_current_back_buffer")]
    internal static extern UIntPtr SwapChainGetCurrentBackBuffer(IntPtr swapChain, out NativeRhiTextureHandle outTexture);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_present")]
    internal static extern UIntPtr SwapChainPresent(IntPtr swapChain);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_reset_suggested")]
    internal static extern UIntPtr SwapChainResetSuggested(IntPtr swapChain, out int outSuggested);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_swap_chain_reset")]
    internal static extern UIntPtr SwapChainReset(IntPtr swapChain, in NativeSwapChainDesc desc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_get_command_queue_index")]
    internal static extern UIntPtr CommandBufferGetCommandQueueIndex(IntPtr commandBuffer, out uint outIndex);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_reset")]
    internal static extern UIntPtr CommandBufferReset(IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_attach_device_object")]
    internal static extern UIntPtr CommandBufferAttachDeviceObject(IntPtr commandBuffer, IntPtr deviceObject);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_begin_event")]
    internal static extern UIntPtr CommandBufferBeginEvent(
        IntPtr commandBuffer,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string eventName);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_end_event")]
    internal static extern UIntPtr CommandBufferEndEvent(IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_resource_barrier")]
    internal static extern UIntPtr CommandBufferResourceBarrier(
        IntPtr commandBuffer,
        [In] NativeBufferBarrier[]? bufferBarriers,
        ulong bufferBarrierCount,
        [In] NativeTextureBarrier[]? textureBarriers,
        ulong textureBarrierCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_begin_render_pass")]
    internal static extern UIntPtr CommandBufferBeginRenderPassNoDepth(
        IntPtr commandBuffer,
        [In] NativeColorAttachment[]? colorAttachments,
        uint colorAttachmentCount,
        [In] NativeResolveAttachment[]? resolveAttachments,
        uint resolveAttachmentCount,
        IntPtr depthStencilAttachment,
        IntPtr occlusionQueryHeap,
        IntPtr timestampQueryHeap,
        IntPtr pipelineStatisticsQueryHeap,
        uint timestampQueryBeginPassWriteIndex,
        uint timestampQueryEndPassWriteIndex,
        uint pipelineStatisticsQueryWriteIndex,
        uint arraySize,
        byte sampleCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_begin_render_pass")]
    internal static extern UIntPtr CommandBufferBeginRenderPassWithDepth(
        IntPtr commandBuffer,
        [In] NativeColorAttachment[]? colorAttachments,
        uint colorAttachmentCount,
        [In] NativeResolveAttachment[]? resolveAttachments,
        uint resolveAttachmentCount,
        in NativeDepthStencilAttachment depthStencilAttachment,
        IntPtr occlusionQueryHeap,
        IntPtr timestampQueryHeap,
        IntPtr pipelineStatisticsQueryHeap,
        uint timestampQueryBeginPassWriteIndex,
        uint timestampQueryEndPassWriteIndex,
        uint pipelineStatisticsQueryWriteIndex,
        uint arraySize,
        byte sampleCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_graphics_pipeline_layout")]
    internal static extern UIntPtr CommandBufferSetGraphicsPipelineLayout(IntPtr commandBuffer, IntPtr pipelineLayout);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_graphics_pipeline_state")]
    internal static extern UIntPtr CommandBufferSetGraphicsPipelineState(IntPtr commandBuffer, IntPtr pipelineState);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_graphics_descriptor_set")]
    internal static extern UIntPtr CommandBufferSetGraphicsDescriptorSet(IntPtr commandBuffer, uint index, IntPtr descriptorSet);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_graphics_descriptor_sets")]
    internal static extern UIntPtr CommandBufferSetGraphicsDescriptorSets(
        IntPtr commandBuffer,
        uint startIndex,
        [In] IntPtr[]? descriptorSets,
        ulong descriptorSetCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_vertex_buffer")]
    internal static extern UIntPtr CommandBufferSetVertexBuffer(IntPtr commandBuffer, uint slot, IntPtr buffer, ulong offset, uint size, uint elementSize);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_vertex_buffers")]
    internal static extern UIntPtr CommandBufferSetVertexBuffers(
        IntPtr commandBuffer,
        uint startSlot,
        [In] NativeVertexBufferView[]? views,
        ulong viewCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_index_buffer")]
    internal static extern UIntPtr CommandBufferSetIndexBuffer(IntPtr commandBuffer, IntPtr buffer, ulong offset, uint size, uint format);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_viewport")]
    internal static extern UIntPtr CommandBufferSetViewport(IntPtr commandBuffer, float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_viewports")]
    internal static extern UIntPtr CommandBufferSetViewports(
        IntPtr commandBuffer,
        [In] NativeViewport[]? viewports,
        ulong viewportCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_scissor_rect")]
    internal static extern UIntPtr CommandBufferSetScissorRect(IntPtr commandBuffer, int offsetX, int offsetY, int width, int height);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_scissor_rects")]
    internal static extern UIntPtr CommandBufferSetScissorRects(
        IntPtr commandBuffer,
        [In] NativeRhiRectI[]? rects,
        ulong rectCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_blend_factor")]
    internal static extern UIntPtr CommandBufferSetBlendFactor(IntPtr commandBuffer, float red, float green, float blue, float alpha);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_stencil_ref")]
    internal static extern UIntPtr CommandBufferSetStencilRef(IntPtr commandBuffer, uint stencilRef);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_draw")]
    internal static extern UIntPtr CommandBufferDraw(IntPtr commandBuffer, uint vertexCount, uint startVertexLocation);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_draw_indexed")]
    internal static extern UIntPtr CommandBufferDrawIndexed(IntPtr commandBuffer, uint indexCount, uint startIndexLocation, int baseVertexLocation);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_draw_instanced")]
    internal static extern UIntPtr CommandBufferDrawInstanced(
        IntPtr commandBuffer,
        uint vertexCountPerInstance,
        uint instanceCount,
        uint startVertexLocation,
        uint startInstanceLocation);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_draw_indexed_instanced")]
    internal static extern UIntPtr CommandBufferDrawIndexedInstanced(
        IntPtr commandBuffer,
        uint indexCountPerInstance,
        uint instanceCount,
        uint startIndexLocation,
        int baseVertexLocation,
        uint startInstanceLocation);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_begin_occlusion_query")]
    internal static extern UIntPtr CommandBufferBeginOcclusionQuery(IntPtr commandBuffer, uint mode, uint index);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_end_occlusion_query")]
    internal static extern UIntPtr CommandBufferEndOcclusionQuery(IntPtr commandBuffer, uint index);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_end_render_pass")]
    internal static extern UIntPtr CommandBufferEndRenderPass(IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_begin_compute_pass")]
    internal static extern UIntPtr CommandBufferBeginComputePass(IntPtr commandBuffer, in NativeComputePassDesc desc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_compute_pipeline_layout")]
    internal static extern UIntPtr CommandBufferSetComputePipelineLayout(IntPtr commandBuffer, IntPtr pipelineLayout);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_compute_pipeline_state")]
    internal static extern UIntPtr CommandBufferSetComputePipelineState(IntPtr commandBuffer, IntPtr pipelineState);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_compute_descriptor_set")]
    internal static extern UIntPtr CommandBufferSetComputeDescriptorSet(IntPtr commandBuffer, uint index, IntPtr descriptorSet);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_set_compute_descriptor_sets")]
    internal static extern UIntPtr CommandBufferSetComputeDescriptorSets(
        IntPtr commandBuffer,
        uint startIndex,
        [In] IntPtr[]? descriptorSets,
        ulong descriptorSetCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_dispatch")]
    internal static extern UIntPtr CommandBufferDispatch(IntPtr commandBuffer, uint threadGroupCountX, uint threadGroupCountY, uint threadGroupCountZ);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_end_compute_pass")]
    internal static extern UIntPtr CommandBufferEndComputePass(IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_begin_copy_pass")]
    internal static extern UIntPtr CommandBufferBeginCopyPass(IntPtr commandBuffer, in NativeCopyPassDesc desc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_copy_resource")]
    internal static extern UIntPtr CommandBufferCopyResource(IntPtr commandBuffer, IntPtr destinationObject, IntPtr sourceObject);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_copy_buffer")]
    internal static extern UIntPtr CommandBufferCopyBuffer(
        IntPtr commandBuffer,
        IntPtr destination,
        ulong destinationOffset,
        IntPtr source,
        ulong sourceOffset,
        ulong copyBytes);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_copy_texture")]
    internal static extern UIntPtr CommandBufferCopyTexture(
        IntPtr commandBuffer,
        IntPtr destination,
        NativeSubresourceIndex destinationSubresource,
        uint destinationX,
        uint destinationY,
        uint destinationZ,
        IntPtr source,
        NativeSubresourceIndex sourceSubresource,
        uint sourceX,
        uint sourceY,
        uint sourceZ,
        uint copyWidth,
        uint copyHeight,
        uint copyDepth);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_copy_buffer_to_texture")]
    internal static extern UIntPtr CommandBufferCopyBufferToTexture(
        IntPtr commandBuffer,
        IntPtr destination,
        NativeSubresourceIndex destinationSubresource,
        uint destinationX,
        uint destinationY,
        uint destinationZ,
        IntPtr source,
        ulong sourceOffset,
        uint sourceRowPitch,
        uint sourceSlicePitch,
        uint copyWidth,
        uint copyHeight,
        uint copyDepth);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_copy_texture_to_buffer")]
    internal static extern UIntPtr CommandBufferCopyTextureToBuffer(
        IntPtr commandBuffer,
        IntPtr destination,
        ulong destinationOffset,
        uint destinationRowPitch,
        uint destinationSlicePitch,
        IntPtr source,
        NativeSubresourceIndex sourceSubresource,
        uint sourceX,
        uint sourceY,
        uint sourceZ,
        uint copyWidth,
        uint copyHeight,
        uint copyDepth);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_end_copy_pass")]
    internal static extern UIntPtr CommandBufferEndCopyPass(IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_submit")]
    internal static extern UIntPtr CommandBufferSubmit(IntPtr commandBuffer, int allowHostWaiting);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_submit_with_fences")]
    internal static extern UIntPtr CommandBufferSubmitWithFences(
        IntPtr commandBuffer,
        [In] IntPtr[]? waitFences,
        ulong waitFenceCount,
        [In] IntPtr[]? signalFences,
        ulong signalFenceCount,
        int allowHostWaiting);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_wait")]
    internal static extern UIntPtr CommandBufferWait(IntPtr commandBuffer);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_command_buffer_try_wait")]
    internal static extern UIntPtr CommandBufferTryWait(IntPtr commandBuffer, out int outWaited);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_buffer_get_desc")]
    internal static extern UIntPtr BufferGetDesc(IntPtr buffer, out NativeBufferDesc outDesc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_buffer_map")]
    internal static extern UIntPtr BufferMap(IntPtr buffer, ulong readBegin, ulong readEnd, out IntPtr outData);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_buffer_unmap")]
    internal static extern UIntPtr BufferUnmap(IntPtr buffer, ulong writeBegin, ulong writeEnd);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_resource_get_memory")]
    internal static extern UIntPtr ResourceGetMemory(IntPtr resourceObject, out NativeRhiDeviceMemoryHandle outMemory);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_memory_get_memory_type")]
    internal static extern UIntPtr DeviceMemoryGetMemoryType(IntPtr deviceMemory, out uint outMemoryType);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_device_memory_get_size")]
    internal static extern UIntPtr DeviceMemoryGetSize(IntPtr deviceMemory, out ulong outSize);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_query_heap_get_desc")]
    internal static extern UIntPtr QueryHeapGetDesc(IntPtr queryHeap, out NativeQueryHeapDesc outDesc);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_query_heap_get_timestamp_values")]
    internal static extern UIntPtr QueryHeapGetTimestampValues(
        IntPtr queryHeap,
        uint index,
        uint count,
        [Out] ulong[] values);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_query_heap_get_occlusion_values")]
    internal static extern UIntPtr QueryHeapGetOcclusionValues(
        IntPtr queryHeap,
        uint index,
        uint count,
        [Out] ulong[] values);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_query_heap_get_pipeline_statistics_values")]
    internal static extern UIntPtr QueryHeapGetPipelineStatisticsValues(
        IntPtr queryHeap,
        uint index,
        uint count,
        [Out] NativePipelineStatistics[] values);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_descriptor_set_update_buffer_view")]
    internal static extern UIntPtr DescriptorSetUpdateBufferView(
        IntPtr descriptorSet,
        uint bindingSlot,
        uint firstArrayIndex,
        uint descriptorType,
        [In] NativeBufferViewDesc[]? views,
        uint viewCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_descriptor_set_update_texture_view")]
    internal static extern UIntPtr DescriptorSetUpdateTextureView(
        IntPtr descriptorSet,
        uint bindingSlot,
        uint firstArrayIndex,
        uint descriptorType,
        [In] NativeTextureViewDesc[]? views,
        uint viewCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_descriptor_set_update_sampler")]
    internal static extern UIntPtr DescriptorSetUpdateSampler(
        IntPtr descriptorSet,
        uint bindingSlot,
        uint firstArrayIndex,
        [In] NativeSamplerDesc[]? samplers,
        uint samplerCount);

    [DllImport(LibraryName, EntryPoint = "luna_rhi_texture_get_desc")]
    internal static extern UIntPtr TextureGetDesc(IntPtr texture, out NativeTextureDesc outDesc);
}
