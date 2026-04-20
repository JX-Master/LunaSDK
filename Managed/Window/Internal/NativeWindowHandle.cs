using System;
using System.Runtime.InteropServices;

namespace Luna.Window.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeWindowHandle
{
    public readonly IntPtr Object;

    public readonly IntPtr IWindow;
}
