namespace Luna.RHI;

public readonly struct InputBindingDesc
{
    public InputBindingDesc(uint bindingSlot, uint elementSize, InputRate inputRate)
    {
        BindingSlot = bindingSlot;
        ElementSize = elementSize;
        InputRate = inputRate;
    }

    public uint BindingSlot { get; }

    public uint ElementSize { get; }

    public InputRate InputRate { get; }
}
