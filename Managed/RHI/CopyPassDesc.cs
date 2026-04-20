namespace Luna.RHI;

public sealed class CopyPassDesc
{
    public IQueryHeap? TimestampQueryHeap { get; init; }

    public uint TimestampQueryBeginPassWriteIndex { get; init; } = uint.MaxValue;

    public uint TimestampQueryEndPassWriteIndex { get; init; } = uint.MaxValue;
}
