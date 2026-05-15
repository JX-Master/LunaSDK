using System;
using System.Runtime.InteropServices;

namespace Luna.AHI.Internal;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate uint NativePlaybackDataCallback(IntPtr destinationBuffer, NativeWaveFormat format, uint numFrames, IntPtr userdata);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void NativeCaptureDataCallback(IntPtr sourceBuffer, NativeWaveFormat format, uint numFrames, IntPtr userdata);
