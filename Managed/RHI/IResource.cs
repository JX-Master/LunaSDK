namespace Luna.RHI;

public interface IResource : IDeviceChild
{
    IDeviceMemory Memory { get; }
}
