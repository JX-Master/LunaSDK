using Luna.Runtime;

namespace Luna.AHI;

public delegate uint PlaybackDataCallback(System.IntPtr destinationBuffer, WaveFormat format, uint numFrames);

public delegate void CaptureDataCallback(System.IntPtr sourceBuffer, WaveFormat format, uint numFrames);

public interface IDevice : IObject
{
    uint SampleRate { get; }

    DeviceFlag Flags { get; }

    uint PlaybackNumChannels { get; }

    BitDepth PlaybackBitDepth { get; }

    uint CaptureNumChannels { get; }

    BitDepth CaptureBitDepth { get; }

    ulong AddPlaybackDataCallback(PlaybackDataCallback callback);

    void RemovePlaybackDataCallback(ulong handle);

    ulong AddCaptureDataCallback(CaptureDataCallback callback);

    void RemoveCaptureDataCallback(ulong handle);
}
