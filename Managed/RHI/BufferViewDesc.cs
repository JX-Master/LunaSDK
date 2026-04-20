namespace Luna.RHI;

public readonly struct BufferViewDesc
{
    public static BufferViewDesc UniformBuffer(IBuffer buffer, ulong offset = 0, uint size = uint.MaxValue)
    {
        return new BufferViewDesc(offset, buffer, 1, size);
    }

    public static BufferViewDesc StructuredBuffer(IBuffer buffer, ulong firstElement, uint elementCount, uint elementSize)
    {
        return new BufferViewDesc(firstElement, buffer, elementCount, elementSize);
    }

    public BufferViewDesc(ulong firstElement, IBuffer buffer, uint elementCount, uint elementSize)
    {
        FirstElement = firstElement;
        Buffer = buffer;
        ElementCount = elementCount;
        ElementSize = elementSize;
    }

    public ulong FirstElement { get; }

    public IBuffer Buffer { get; }

    public uint ElementCount { get; }

    public uint ElementSize { get; }
}
