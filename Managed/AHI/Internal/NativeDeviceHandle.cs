using System;
using System.Runtime.InteropServices;

namespace Luna.AHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeDeviceHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr IDevice;
}
