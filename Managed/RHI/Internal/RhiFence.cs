using System;

namespace Luna.RHI;

internal sealed class RhiFence : RhiDeviceChild, IFence
{
    private readonly IntPtr _ifence;

    internal RhiFence(IntPtr nativeObject, IntPtr nativeFence, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeFence == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeFence));
        }
        _ifence = nativeFence;
    }

    internal static IntPtr GetNativeFencePointer(IFence fence)
    {
        ArgumentNullException.ThrowIfNull(fence);
        if (fence is not RhiFence nativeFence)
        {
            throw new ArgumentException("The fence must be created by Luna.RHI.", nameof(fence));
        }
        nativeFence.EnsureNotDisposed();
        return nativeFence._ifence;
    }
}
