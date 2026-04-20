namespace Luna.RHI;

public interface IDeviceMemory : IDeviceChild
{
    MemoryType MemoryType { get; }

    ulong Size { get; }
}
