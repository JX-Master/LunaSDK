using Luna.RHI;

namespace Luna.RHIUtility;

public interface IMipmapGenerationContext : IDeviceChild
{
    void Reset();

    void GenerateMipmaps(ITexture texture, uint sourceMip = 0, uint numGenerateMips = uint.MaxValue);

    void Commit(ICommandBuffer commandBuffer, bool submitAndWait);
}
