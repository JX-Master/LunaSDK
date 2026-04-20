using Luna.RHI;

namespace Luna.RHIUtility;

public interface IResourceReadContext : IDeviceChild
{
    void Reset();

    ulong ReadBuffer(IBuffer buffer, ulong offset, ulong size);

    ulong ReadTexture(
        ITexture texture,
        SubresourceIndex subresource,
        uint x,
        uint y,
        uint z,
        uint width,
        uint height,
        uint depth);

    void Commit(ICommandBuffer commandBuffer, bool submitAndWait);

    byte[] GetBufferData(ulong handle, ulong size);

    TextureReadData GetTextureData(ulong handle, uint copyBytesPerRow, uint height, uint depth);
}
