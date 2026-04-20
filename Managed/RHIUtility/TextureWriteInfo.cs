namespace Luna.RHIUtility;

public readonly struct TextureWriteInfo
{
    public TextureWriteInfo(uint rowPitch, uint slicePitch)
    {
        RowPitch = rowPitch;
        SlicePitch = slicePitch;
    }

    public uint RowPitch { get; }

    public uint SlicePitch { get; }
}
