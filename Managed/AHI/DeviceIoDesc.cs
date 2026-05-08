namespace Luna.AHI;

public readonly struct DeviceIoDesc
{
    public DeviceIoDesc(IAdapter? adapter, uint numChannels, BitDepth bitDepth)
    {
        Adapter = adapter;
        NumChannels = numChannels;
        BitDepth = bitDepth;
    }

    public IAdapter? Adapter { get; init; }

    public uint NumChannels { get; init; }

    public BitDepth BitDepth { get; init; }
}
