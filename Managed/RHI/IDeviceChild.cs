using Luna.Runtime;

namespace Luna.RHI;

public interface IDeviceChild : IObject
{
    IDevice Device { get; }

    void SetName(string name);
}
