using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal abstract class RhiDeviceChild : ObjectBase, IDeviceChild
{
    protected RhiDeviceChild(IntPtr nativeObject, bool retain)
        : base(nativeObject, retain)
    {
    }

    public IDevice Device
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceChildGetDevice(GetNativeHandle(), out var device)));
            return new RhiDevice(device.Object, device.IDevice, retain: true);
        }
    }

    public void SetName(string name)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(name);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceChildSetName(GetNativeHandle(), name)));
    }
}
