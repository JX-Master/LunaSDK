using System;
using System.Runtime.InteropServices;

namespace Luna.RHIUtility.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiUtilityResourceReadContextHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IResourceReadContext;
}
