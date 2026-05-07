using System;
using Microsoft.Win32.SafeHandles;

namespace Luna.VG.Internal;

internal sealed class NativeTextArrangeResultHandle : SafeHandleZeroOrMinusOneIsInvalid
{
    internal NativeTextArrangeResultHandle(IntPtr handle)
        : base(true)
    {
        SetHandle(handle);
    }

    protected override bool ReleaseHandle()
    {
        VgNative.TextArrangeResultFree(handle);
        return true;
    }
}
