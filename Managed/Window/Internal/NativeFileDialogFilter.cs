using System;
using System.Runtime.InteropServices;

namespace Luna.Window.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeFileDialogFilter
{
    public NativeFileDialogFilter(IntPtr name, IntPtr extensions, ulong extensionCount)
    {
        Name = name;
        Extensions = extensions;
        ExtensionCount = extensionCount;
    }

    public readonly IntPtr Name;
    public readonly IntPtr Extensions;
    public readonly ulong ExtensionCount;
}
