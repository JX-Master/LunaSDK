namespace Luna.RHI;

public readonly struct DescriptorSetDesc
{
    public DescriptorSetDesc(IDescriptorSetLayout layout, uint numVariableDescriptors = 0)
    {
        Layout = layout;
        NumVariableDescriptors = numVariableDescriptors;
    }

    public IDescriptorSetLayout Layout { get; }

    public uint NumVariableDescriptors { get; }
}
