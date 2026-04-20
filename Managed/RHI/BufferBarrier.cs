namespace Luna.RHI;

public readonly struct BufferBarrier
{
    public BufferBarrier(
        IBuffer buffer,
        BufferStateFlags before,
        BufferStateFlags after,
        ResourceBarrierFlags flags = ResourceBarrierFlags.None)
    {
        Buffer = buffer;
        Before = before;
        After = after;
        Flags = flags;
    }

    public IBuffer Buffer { get; }

    public BufferStateFlags Before { get; }

    public BufferStateFlags After { get; }

    public ResourceBarrierFlags Flags { get; }
}
