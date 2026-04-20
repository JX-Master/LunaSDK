using System;
using Luna.Runtime;

namespace Luna.Window;

internal sealed class NativeWindowEvent : ObjectBase
{
    internal NativeWindowEvent(IntPtr nativeObject, bool retain)
        : base(nativeObject, retain)
    {
    }

    internal IntPtr EventObject => GetNativeHandle();
}
