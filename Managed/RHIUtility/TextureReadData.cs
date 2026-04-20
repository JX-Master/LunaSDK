namespace Luna.RHIUtility;

public sealed class TextureReadData
{
    public TextureReadData(byte[] data, uint rowPitch, uint slicePitch)
    {
        Data = data;
        RowPitch = rowPitch;
        SlicePitch = slicePitch;
    }

    public byte[] Data { get; }

    public uint RowPitch { get; }

    public uint SlicePitch { get; }
}
