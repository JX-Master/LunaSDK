using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFileHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IFile;

    public readonly IntPtr ISeekableStream;

    public readonly IntPtr IStream;
}
