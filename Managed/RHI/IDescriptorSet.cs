namespace Luna.RHI;

public interface IDescriptorSet : IDeviceChild
{
    void SetUniformBufferView(uint bindingSlot, BufferViewDesc view);

    void SetUniformBufferViews(uint bindingSlot, uint firstArrayIndex, BufferViewDesc[] views);

    void SetReadBufferView(uint bindingSlot, BufferViewDesc view);

    void SetReadBufferViews(uint bindingSlot, uint firstArrayIndex, BufferViewDesc[] views);

    void SetReadWriteBufferView(uint bindingSlot, BufferViewDesc view);

    void SetReadWriteBufferViews(uint bindingSlot, uint firstArrayIndex, BufferViewDesc[] views);

    void SetReadTextureView(uint bindingSlot, TextureViewDesc view);

    void SetReadTextureViews(uint bindingSlot, uint firstArrayIndex, TextureViewDesc[] views);

    void SetReadWriteTextureView(uint bindingSlot, TextureViewDesc view);

    void SetReadWriteTextureViews(uint bindingSlot, uint firstArrayIndex, TextureViewDesc[] views);

    void SetSampler(uint bindingSlot, SamplerDesc sampler);

    void SetSamplers(uint bindingSlot, uint firstArrayIndex, SamplerDesc[] samplers);
}
