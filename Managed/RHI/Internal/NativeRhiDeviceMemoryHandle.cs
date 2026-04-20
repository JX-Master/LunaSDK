using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiDeviceMemoryHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IDeviceMemory;
}
