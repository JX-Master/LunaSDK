using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeCopyPassDesc
{
    public readonly IntPtr TimestampQueryHeap;

    public readonly uint TimestampQueryBeginPassWriteIndex;

    public readonly uint TimestampQueryEndPassWriteIndex;

    internal NativeCopyPassDesc(CopyPassDesc desc)
    {
        TimestampQueryHeap = desc.TimestampQueryHeap is null ? IntPtr.Zero : RhiQueryHeap.GetNativeQueryHeapPointer(desc.TimestampQueryHeap);
        TimestampQueryBeginPassWriteIndex = desc.TimestampQueryBeginPassWriteIndex;
        TimestampQueryEndPassWriteIndex = desc.TimestampQueryEndPassWriteIndex;
    }
}
