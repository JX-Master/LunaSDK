using System;
using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeComputePassDesc
{
    public readonly IntPtr TimestampQueryHeap;

    public readonly IntPtr PipelineStatisticsQueryHeap;

    public readonly uint TimestampQueryBeginPassWriteIndex;

    public readonly uint TimestampQueryEndPassWriteIndex;

    public readonly uint PipelineStatisticsQueryWriteIndex;

    internal NativeComputePassDesc(ComputePassDesc desc)
    {
        TimestampQueryHeap = desc.TimestampQueryHeap is null ? IntPtr.Zero : RhiQueryHeap.GetNativeQueryHeapPointer(desc.TimestampQueryHeap);
        PipelineStatisticsQueryHeap = desc.PipelineStatisticsQueryHeap is null ? IntPtr.Zero : RhiQueryHeap.GetNativeQueryHeapPointer(desc.PipelineStatisticsQueryHeap);
        TimestampQueryBeginPassWriteIndex = desc.TimestampQueryBeginPassWriteIndex;
        TimestampQueryEndPassWriteIndex = desc.TimestampQueryEndPassWriteIndex;
        PipelineStatisticsQueryWriteIndex = desc.PipelineStatisticsQueryWriteIndex;
    }
}
