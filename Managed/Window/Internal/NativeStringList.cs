using System;
using System.Runtime.InteropServices;

namespace Luna.Window.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeStringList
{
    public readonly IntPtr Items;
    public readonly ulong Count;
}
