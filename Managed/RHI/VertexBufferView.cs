namespace Luna.RHI;

public readonly struct VertexBufferView
{
    public VertexBufferView(IBuffer buffer, ulong offset, uint size, uint elementSize)
    {
        Buffer = buffer;
        Offset = offset;
        Size = size;
        ElementSize = elementSize;
    }

    public IBuffer Buffer { get; }

    public ulong Offset { get; }

    public uint Size { get; }

    public uint ElementSize { get; }
}
