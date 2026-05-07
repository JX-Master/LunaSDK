namespace Luna.RHI;

public readonly struct InputAttributeDesc
{
    public InputAttributeDesc(uint location, uint bindingSlot, uint offset, Format format)
    {
        Location = location;
        BindingSlot = bindingSlot;
        Offset = offset;
        Format = format;
    }

    public uint Location { get; }

    public uint BindingSlot { get; }

    public uint Offset { get; }

    public Format Format { get; }
}
