namespace Luna.RHI;

public sealed class PipelineLayoutDesc
{
    public IDescriptorSetLayout[] DescriptorSetLayouts { get; init; } = System.Array.Empty<IDescriptorSetLayout>();

    public PipelineLayoutFlags Flags { get; init; } = PipelineLayoutFlags.None;
}
