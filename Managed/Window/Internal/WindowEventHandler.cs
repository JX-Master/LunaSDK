using System;
using System.Runtime.InteropServices;

namespace Luna.Window.Internal;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void WindowEventHandler(IntPtr eventObject, IntPtr userdata);
