namespace Luna.RHI;

public readonly struct DeviceFeatureData
{
    internal DeviceFeatureData(ulong rawValue)
    {
        RawValue = rawValue;
    }

    public ulong RawValue { get; }

    public bool AsBoolean => RawValue != 0;

    public uint AsUInt32 => checked((uint)RawValue);
}
