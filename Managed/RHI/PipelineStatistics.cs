namespace Luna.RHI;

public readonly struct PipelineStatistics
{
    public PipelineStatistics(
        ulong vertexShaderInvocations,
        ulong rasterizerInputPrimitives,
        ulong renderedPrimitives,
        ulong pixelShaderInvocations,
        ulong computeShaderInvocations)
    {
        VertexShaderInvocations = vertexShaderInvocations;
        RasterizerInputPrimitives = rasterizerInputPrimitives;
        RenderedPrimitives = renderedPrimitives;
        PixelShaderInvocations = pixelShaderInvocations;
        ComputeShaderInvocations = computeShaderInvocations;
    }

    public ulong VertexShaderInvocations { get; }

    public ulong RasterizerInputPrimitives { get; }

    public ulong RenderedPrimitives { get; }

    public ulong PixelShaderInvocations { get; }

    public ulong ComputeShaderInvocations { get; }
}
