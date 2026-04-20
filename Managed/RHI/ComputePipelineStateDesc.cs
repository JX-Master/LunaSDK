namespace Luna.RHI;

public sealed class ComputePipelineStateDesc
{
    public IPipelineLayout? PipelineLayout { get; init; }

    public ShaderData ComputeShader { get; init; }

    public uint MetalNumThreadsX { get; init; }

    public uint MetalNumThreadsY { get; init; }

    public uint MetalNumThreadsZ { get; init; }
}
