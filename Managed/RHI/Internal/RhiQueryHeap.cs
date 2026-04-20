using System;
using Luna.Runtime;
using Luna.RHI.Internal;

namespace Luna.RHI;

internal sealed class RhiQueryHeap : RhiDeviceChild, IQueryHeap
{
    private readonly IntPtr _iqueryHeap;

    internal RhiQueryHeap(IntPtr nativeObject, IntPtr nativeQueryHeap, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeQueryHeap == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeQueryHeap));
        }
        _iqueryHeap = nativeQueryHeap;
    }

    public QueryHeapDesc Desc
    {
        get
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.QueryHeapGetDesc(_iqueryHeap, out var desc)));
            return desc.ToPublic();
        }
    }

    public ulong[] GetTimestampValues(uint index, uint count)
    {
        EnsureNotDisposed();
        var values = new ulong[count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.QueryHeapGetTimestampValues(_iqueryHeap, index, count, values)));
        return values;
    }

    public ulong[] GetOcclusionValues(uint index, uint count)
    {
        EnsureNotDisposed();
        var values = new ulong[count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.QueryHeapGetOcclusionValues(_iqueryHeap, index, count, values)));
        return values;
    }

    public PipelineStatistics[] GetPipelineStatisticsValues(uint index, uint count)
    {
        EnsureNotDisposed();
        var nativeValues = new NativePipelineStatistics[count];
        RuntimeErrors.ThrowIfFailed(new ErrorCode(RhiNative.QueryHeapGetPipelineStatisticsValues(_iqueryHeap, index, count, nativeValues)));
        var values = new PipelineStatistics[count];
        for (var i = 0; i < values.Length; ++i)
        {
            values[i] = nativeValues[i].ToPublic();
        }
        return values;
    }

    internal static IntPtr GetNativeQueryHeapPointer(IQueryHeap queryHeap)
    {
        ArgumentNullException.ThrowIfNull(queryHeap);
        if (queryHeap is not RhiQueryHeap nativeQueryHeap)
        {
            throw new ArgumentException("The query heap must be created by Luna.RHI.", nameof(queryHeap));
        }
        nativeQueryHeap.EnsureNotDisposed();
        return nativeQueryHeap._iqueryHeap;
    }
}
