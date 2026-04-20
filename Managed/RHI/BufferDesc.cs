namespace Luna.RHI;

public readonly struct BufferDesc
{
    public BufferDesc(BufferUsageFlags usages, ulong size, ResourceFlags flags = ResourceFlags.None)
    {
        Size = size;
        Usages = usages;
        Flags = flags;
    }

    public ulong Size { get; }

    public BufferUsageFlags Usages { get; }

    public ResourceFlags Flags { get; }
}
