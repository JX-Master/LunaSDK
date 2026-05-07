using System;
using System.Runtime.InteropServices;

namespace Luna.Font.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFontHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IFontFile;
}
