using System;
using System.Runtime.InteropServices;

namespace Luna.Window.Internal;

internal readonly struct PinnedArray<T> : IDisposable
    where T : struct
{
    private readonly GCHandle _handle;

    private PinnedArray(GCHandle handle, IntPtr pointer)
    {
        _handle = handle;
        Pointer = pointer;
    }

    public IntPtr Pointer { get; }

    public static PinnedArray<T> Create(T[] values)
    {
        if (values.Length == 0)
        {
            return new PinnedArray<T>(default, IntPtr.Zero);
        }
        var handle = GCHandle.Alloc(values, GCHandleType.Pinned);
        return new PinnedArray<T>(handle, handle.AddrOfPinnedObject());
    }

    public void Dispose()
    {
        if (_handle.IsAllocated)
        {
            _handle.Free();
        }
    }
}
