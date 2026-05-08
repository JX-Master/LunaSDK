using System.Runtime.InteropServices;

namespace Luna.AHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeWaveFormat
{
    public readonly uint SampleRate;
    public readonly uint NumChannels;
    public readonly byte BitDepth;

    public WaveFormat ToPublic()
    {
        return new WaveFormat(SampleRate, NumChannels, (global::Luna.AHI.BitDepth)BitDepth);
    }
}
