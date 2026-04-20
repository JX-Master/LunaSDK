namespace Luna.RHI;

public interface IQueryHeap : IDeviceChild
{
    QueryHeapDesc Desc { get; }

    ulong[] GetTimestampValues(uint index, uint count);

    ulong[] GetOcclusionValues(uint index, uint count);

    PipelineStatistics[] GetPipelineStatisticsValues(uint index, uint count);
}
