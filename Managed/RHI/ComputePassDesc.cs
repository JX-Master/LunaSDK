namespace Luna.RHI;

public sealed class ComputePassDesc
{
    public IQueryHeap? TimestampQueryHeap { get; init; }

    public IQueryHeap? PipelineStatisticsQueryHeap { get; init; }

    public uint TimestampQueryBeginPassWriteIndex { get; init; } = uint.MaxValue;

    public uint TimestampQueryEndPassWriteIndex { get; init; } = uint.MaxValue;

    public uint PipelineStatisticsQueryWriteIndex { get; init; } = uint.MaxValue;
}
