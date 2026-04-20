using Luna.RHI;

namespace Luna.RHIUtility;

public interface IBlitContext : IDeviceChild
{
    void Reset();

    void Blit(
        ITexture destination,
        SubresourceIndex destinationSubresource,
        TextureViewDesc source,
        SamplerDesc sampler,
        BlitPoint topLeft,
        BlitPoint topRight,
        BlitPoint bottomLeft,
        BlitPoint bottomRight);

    void Commit(ICommandBuffer commandBuffer, bool submitAndWait);
}
