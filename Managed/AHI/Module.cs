using System;
using Luna.AHI.Internal;
using Luna.Runtime;

namespace Luna.AHI;

public static class Module
{
    public static void Init()
    {
        if (!global::Luna.Runtime.Runtime.IsInitialized)
        {
            throw new InvalidOperationException("Luna runtime must be initialized before initializing the AHI module.");
        }
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNativeGenerated.InitModule()));
    }

    public static ulong GetFrameSize(BitDepth bitDepth, uint numChannels)
    {
        ulong sampleSize = bitDepth switch
        {
            BitDepth.U8 => 1,
            BitDepth.S16 => 2,
            BitDepth.S24 => 3,
            BitDepth.S32 => 4,
            BitDepth.F32 => 4,
            _ => 0
        };
        return sampleSize * numChannels;
    }

    public static void GetAdapters(out IAdapter[] playbackAdapters, out IAdapter[] captureAdapters)
    {
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNativeGenerated.GetAdapters(IntPtr.Zero, 0, out var playbackCount, IntPtr.Zero, 0, out var captureCount)));

        var nativePlayback = playbackCount == 0 ? Array.Empty<NativeAdapterHandle>() : new NativeAdapterHandle[checked((int)playbackCount)];
        var nativeCapture = captureCount == 0 ? Array.Empty<NativeAdapterHandle>() : new NativeAdapterHandle[checked((int)captureCount)];
        using var pinnedPlayback = PinnedArray<NativeAdapterHandle>.Create(nativePlayback);
        using var pinnedCapture = PinnedArray<NativeAdapterHandle>.Create(nativeCapture);

        RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNativeGenerated.GetAdapters(
            pinnedPlayback.Pointer,
            (ulong)nativePlayback.Length,
            out playbackCount,
            pinnedCapture.Pointer,
            (ulong)nativeCapture.Length,
            out captureCount)));

        playbackAdapters = new IAdapter[nativePlayback.Length];
        for (var i = 0; i < nativePlayback.Length; ++i)
        {
            playbackAdapters[i] = new NativeAdapter(nativePlayback[i], retain: false);
        }

        captureAdapters = new IAdapter[nativeCapture.Length];
        for (var i = 0; i < nativeCapture.Length; ++i)
        {
            captureAdapters[i] = new NativeAdapter(nativeCapture[i], retain: false);
        }
    }

    public static IDevice CreateDevice(DeviceDesc desc)
    {
        var nativeDesc = NativeDeviceDesc.FromPublic(desc);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNativeGenerated.NewDevice(in nativeDesc, out var device)));
        return new NativeDevice(device, retain: false);
    }
}
