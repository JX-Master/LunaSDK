using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime.Internal;

internal sealed class NativeObjectHandle : SafeHandle
{
    public NativeObjectHandle()
        : base(IntPtr.Zero, true)
    {
    }

    public NativeObjectHandle(IntPtr handle)
        : base(IntPtr.Zero, true)
    {
        SetHandle(handle);
    }

    public override bool IsInvalid => handle == IntPtr.Zero;

    protected override bool ReleaseHandle()
    {
        RuntimeNative.ObjectRelease(handle);
        return true;
    }
}
