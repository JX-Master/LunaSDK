using System;
using System.Runtime.InteropServices;

namespace Luna.AHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeDeviceIoDesc
{
    public readonly IntPtr Adapter;
    public readonly uint NumChannels;
    public readonly byte BitDepth;

    public NativeDeviceIoDesc(IntPtr adapter, uint numChannels, byte bitDepth)
    {
        Adapter = adapter;
        NumChannels = numChannels;
        BitDepth = bitDepth;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeDeviceDesc
{
    public readonly NativeDeviceIoDesc Playback;
    public readonly NativeDeviceIoDesc Capture;
    public readonly uint SampleRate;
    public readonly uint Flags;

    public NativeDeviceDesc(NativeDeviceIoDesc playback, NativeDeviceIoDesc capture, uint sampleRate, uint flags)
    {
        Playback = playback;
        Capture = capture;
        SampleRate = sampleRate;
        Flags = flags;
    }

    public static NativeDeviceDesc FromPublic(DeviceDesc desc)
    {
        return new NativeDeviceDesc(
            new NativeDeviceIoDesc(NativeAdapter.GetNativeAdapterPointer(desc.Playback.Adapter), desc.Playback.NumChannels, (byte)desc.Playback.BitDepth),
            new NativeDeviceIoDesc(NativeAdapter.GetNativeAdapterPointer(desc.Capture.Adapter), desc.Capture.NumChannels, (byte)desc.Capture.BitDepth),
            desc.SampleRate,
            (uint)desc.Flags);
    }
}
