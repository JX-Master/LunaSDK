namespace Luna.RHI;

public interface IBuffer : IResource
{
    BufferDesc Desc { get; }

    void Write(ulong offset, byte[] data);
}
