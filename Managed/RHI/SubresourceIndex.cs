namespace Luna.RHI;

public readonly struct SubresourceIndex
{
    public static SubresourceIndex AllSubresources => new(uint.MaxValue, uint.MaxValue);

    public SubresourceIndex(uint mipSlice, uint arraySlice)
    {
        MipSlice = mipSlice;
        ArraySlice = arraySlice;
    }

    public uint MipSlice { get; }

    public uint ArraySlice { get; }
}
