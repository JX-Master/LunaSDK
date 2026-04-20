using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal sealed class RhiDeviceMemory : RhiDeviceChild, IDeviceMemory
{
    private readonly IntPtr _ideviceMemory;

    internal RhiDeviceMemory(IntPtr nativeObject, IntPtr nativeDeviceMemory, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeDeviceMemory == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeDeviceMemory));
        }
        _ideviceMemory = nativeDeviceMemory;
    }

    public MemoryType MemoryType
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceMemoryGetMemoryType(_ideviceMemory, out var memoryType)));
            return (MemoryType)memoryType;
        }
    }

    public ulong Size
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.DeviceMemoryGetSize(_ideviceMemory, out var size)));
            return size;
        }
    }

    internal static IntPtr GetNativeDeviceMemoryPointer(IDeviceMemory memory)
    {
        ArgumentNullException.ThrowIfNull(memory);
        if (memory is not RhiDeviceMemory nativeMemory)
        {
            throw new ArgumentException("The device memory must be created by Luna.RHI.", nameof(memory));
        }
        nativeMemory.EnsureNotDisposed();
        return nativeMemory._ideviceMemory;
    }
}
