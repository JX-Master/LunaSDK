using System;
using System.Runtime.InteropServices;

namespace Luna.VariantUtils.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeVariantHandle
{
    public readonly IntPtr Variant;

    public NativeVariantHandle(IntPtr variant)
    {
        Variant = variant;
    }
}
