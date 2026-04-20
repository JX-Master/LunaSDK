namespace Luna.RHI;

public readonly struct QueryHeapDesc
{
    public QueryHeapDesc(QueryType type, uint count)
    {
        Type = type;
        Count = count;
    }

    public QueryType Type { get; }

    public uint Count { get; }
}
