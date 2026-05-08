using Luna.Runtime;

namespace Luna.AHI;

public interface IAdapter : IObject
{
    string Name { get; }

    bool IsPrimary { get; }

    WaveFormat[] GetNativeWaveFormats();
}
