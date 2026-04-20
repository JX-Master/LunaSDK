using System;

namespace Luna.RHI;

[Flags]
public enum DescriptorSetLayoutFlags : uint
{
    None = 0,
    VariableDescriptors = 1
}
