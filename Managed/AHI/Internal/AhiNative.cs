using System;
using System.Runtime.InteropServices;

namespace Luna.AHI.Internal;

internal static class AhiNative
{
    private const string LibraryName = "LunaAHIC";

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate uint NativePlaybackDataCallback(IntPtr destinationBuffer, NativeWaveFormat format, uint numFrames, IntPtr userdata);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void NativeCaptureDataCallback(IntPtr sourceBuffer, NativeWaveFormat format, uint numFrames, IntPtr userdata);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_init_module")]
    internal static extern UIntPtr InitModule();

    [DllImport(LibraryName, EntryPoint = "luna_ahi_free_string")]
    internal static extern void FreeString(IntPtr text);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_get_adapters")]
    internal static extern UIntPtr GetAdapters(
        [Out] NativeAdapterHandle[]? outPlaybackAdapters,
        ulong playbackCapacity,
        out ulong outPlaybackCount,
        [Out] NativeAdapterHandle[]? outCaptureAdapters,
        ulong captureCapacity,
        out ulong outCaptureCount);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_iadapter_get_name")]
    internal static extern UIntPtr IAdapterGetName(IntPtr self, out IntPtr outName);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_iadapter_is_primary")]
    internal static extern int IAdapterIsPrimary(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_iadapter_get_native_wave_formats")]
    internal static extern UIntPtr IAdapterGetNativeWaveFormats(
        IntPtr self,
        [Out] NativeWaveFormat[]? outFormats,
        ulong capacity,
        out ulong outCount);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_new_device")]
    internal static extern UIntPtr NewDevice(in NativeDeviceDesc desc, out NativeDeviceHandle outDevice);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_get_sample_rate")]
    internal static extern uint IDeviceGetSampleRate(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_get_flags")]
    internal static extern uint IDeviceGetFlags(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_get_playback_num_channels")]
    internal static extern uint IDeviceGetPlaybackNumChannels(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_get_playback_bit_depth")]
    internal static extern byte IDeviceGetPlaybackBitDepth(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_get_capture_num_channels")]
    internal static extern uint IDeviceGetCaptureNumChannels(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_get_capture_bit_depth")]
    internal static extern byte IDeviceGetCaptureBitDepth(IntPtr self);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_add_playback_data_callback")]
    internal static extern UIntPtr IDeviceAddPlaybackDataCallback(
        IntPtr self,
        NativePlaybackDataCallback callback,
        IntPtr userdata,
        out ulong outHandle);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_remove_playback_data_callback")]
    internal static extern void IDeviceRemovePlaybackDataCallback(IntPtr self, ulong handle);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_add_capture_data_callback")]
    internal static extern UIntPtr IDeviceAddCaptureDataCallback(
        IntPtr self,
        NativeCaptureDataCallback callback,
        IntPtr userdata,
        out ulong outHandle);

    [DllImport(LibraryName, EntryPoint = "luna_ahi_idevice_remove_capture_data_callback")]
    internal static extern void IDeviceRemoveCaptureDataCallback(IntPtr self, ulong handle);
}
