namespace Luna.AHI;

public readonly struct WaveFormat
{
    public WaveFormat(uint sampleRate, uint numChannels, BitDepth bitDepth)
    {
        SampleRate = sampleRate;
        NumChannels = numChannels;
        BitDepth = bitDepth;
    }

    public uint SampleRate { get; init; }

    public uint NumChannels { get; init; }

    public BitDepth BitDepth { get; init; }
}
