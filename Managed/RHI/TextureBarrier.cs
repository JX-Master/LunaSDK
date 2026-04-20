namespace Luna.RHI;

public readonly struct TextureBarrier
{
    public TextureBarrier(
        ITexture texture,
        SubresourceIndex subresource,
        TextureStateFlags before,
        TextureStateFlags after,
        ResourceBarrierFlags flags = ResourceBarrierFlags.None)
    {
        Texture = texture;
        Subresource = subresource;
        Before = before;
        After = after;
        Flags = flags;
    }

    public ITexture Texture { get; }

    public SubresourceIndex Subresource { get; }

    public TextureStateFlags Before { get; }

    public TextureStateFlags After { get; }

    public ResourceBarrierFlags Flags { get; }
}
