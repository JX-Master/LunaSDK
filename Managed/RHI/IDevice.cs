using Luna.Runtime;

namespace Luna.RHI;

public interface IDevice : IObject
{
    uint CommandQueueCount { get; }

    CommandQueueDesc GetCommandQueueDesc(uint index);

    double GetCommandQueueTimestampFrequency(uint index);

    DeviceFeatureData CheckFeature(DeviceFeature feature);

    TextureDataPlacementInfo GetTextureDataPlacementInfo(uint width, uint height, uint depth, Format format);

    bool IsResourcesAliasingCompatible(MemoryType memoryType, BufferDesc[] buffers, TextureDesc[] textures);

    IDeviceMemory AllocateMemory(MemoryType memoryType, BufferDesc[] buffers, TextureDesc[] textures);

    IBuffer CreateBuffer(MemoryType memoryType, BufferDesc desc);

    ITexture CreateTexture(MemoryType memoryType, TextureDesc desc);

    ITexture CreateTexture(MemoryType memoryType, TextureDesc desc, ClearValue optimizedClearValue);

    IBuffer CreateAliasingBuffer(IDeviceMemory deviceMemory, BufferDesc desc);

    ITexture CreateAliasingTexture(IDeviceMemory deviceMemory, TextureDesc desc);

    ITexture CreateAliasingTexture(IDeviceMemory deviceMemory, TextureDesc desc, ClearValue optimizedClearValue);

    IFence CreateFence();

    IQueryHeap CreateQueryHeap(QueryHeapDesc desc);

    IDescriptorSetLayout CreateDescriptorSetLayout(DescriptorSetLayoutDesc desc);

    IDescriptorSet CreateDescriptorSet(DescriptorSetDesc desc);

    IPipelineLayout CreatePipelineLayout(PipelineLayoutDesc desc);

    IPipelineState CreateGraphicsPipelineState(GraphicsPipelineStateDesc desc);

    IPipelineState CreateComputePipelineState(ComputePipelineStateDesc desc);

    ISwapChain CreateSwapChain(uint commandQueueIndex, Luna.Window.IWindow window, SwapChainDesc desc);

    ICommandBuffer CreateCommandBuffer(uint commandQueueIndex);
}
