using System;
using System.Runtime.InteropServices;

namespace Luna.RHIUtility.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiUtilityMipmapGenerationContextHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IMipmapGenerationContext;
}
