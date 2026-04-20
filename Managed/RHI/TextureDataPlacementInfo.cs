namespace Luna.RHI;

public readonly struct TextureDataPlacementInfo
{
    public TextureDataPlacementInfo(ulong size, ulong alignment, ulong rowPitch, ulong slicePitch)
    {
        Size = size;
        Alignment = alignment;
        RowPitch = rowPitch;
        SlicePitch = slicePitch;
    }

    public ulong Size { get; }

    public ulong Alignment { get; }

    public ulong RowPitch { get; }

    public ulong SlicePitch { get; }
}
