using System;
using System.Runtime.InteropServices;

namespace Luna.Image.Internal;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeImageData
{
    public IntPtr Data;
    public ulong DataSize;
    public NativeImageDesc Desc;
}
