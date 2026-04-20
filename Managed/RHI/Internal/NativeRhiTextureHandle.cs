using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiTextureHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr ITexture;
}
