using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal abstract class RhiResource : RhiDeviceChild, IResource
{
    protected RhiResource(IntPtr nativeObject, bool retain)
        : base(nativeObject, retain)
    {
    }

    public IDeviceMemory Memory
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.ResourceGetMemory(GetNativeHandle(), out var memory)));
            return new RhiDeviceMemory(memory.Object, memory.IDeviceMemory, retain: false);
        }
    }
}
