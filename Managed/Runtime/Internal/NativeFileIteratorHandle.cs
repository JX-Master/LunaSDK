using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFileIteratorHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IFileIterator;
}
