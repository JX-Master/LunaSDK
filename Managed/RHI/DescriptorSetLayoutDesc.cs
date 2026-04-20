using System;

namespace Luna.RHI;

public sealed class DescriptorSetLayoutDesc
{
    public DescriptorSetLayoutBinding[] Bindings { get; init; } = Array.Empty<DescriptorSetLayoutBinding>();

    public DescriptorSetLayoutFlags Flags { get; init; } = DescriptorSetLayoutFlags.None;
}
