using System;
using System.Runtime.InteropServices;

namespace Luna.AHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeAdapterHandle
{
    public readonly IntPtr Object;
    public readonly IntPtr IAdapter;
}
