namespace Luna.AHI;

public readonly struct DeviceDesc
{
    public DeviceDesc(DeviceIoDesc playback, DeviceIoDesc capture, uint sampleRate, DeviceFlag flags)
    {
        Playback = playback;
        Capture = capture;
        SampleRate = sampleRate;
        Flags = flags;
    }

    public DeviceIoDesc Playback { get; init; }

    public DeviceIoDesc Capture { get; init; }

    public uint SampleRate { get; init; }

    public DeviceFlag Flags { get; init; }
}
