using Luna.RHI;

namespace Luna.RHIUtility;

public interface IResourceWriteContext : IDeviceChild
{
    void Reset();

    void WriteBuffer(IBuffer buffer, ulong offset, byte[] data);

    TextureWriteInfo WriteTexture(
        ITexture texture,
        SubresourceIndex subresource,
        uint x,
        uint y,
        uint z,
        uint width,
        uint height,
        uint depth,
        byte[] data,
        uint sourceRowPitch,
        uint sourceSlicePitch,
        uint copyBytesPerRow);

    void Commit(ICommandBuffer commandBuffer, bool submitAndWait);
}
