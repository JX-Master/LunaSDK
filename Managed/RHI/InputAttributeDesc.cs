namespace Luna.RHI;

public readonly struct InputAttributeDesc
{
    public InputAttributeDesc(string semanticName, uint semanticIndex, uint location, uint bindingSlot, uint offset, Format format)
    {
        SemanticName = semanticName;
        SemanticIndex = semanticIndex;
        Location = location;
        BindingSlot = bindingSlot;
        Offset = offset;
        Format = format;
    }

    public string SemanticName { get; }

    public uint SemanticIndex { get; }

    public uint Location { get; }

    public uint BindingSlot { get; }

    public uint Offset { get; }

    public Format Format { get; }
}
