using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiBufferHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IBuffer;
}
