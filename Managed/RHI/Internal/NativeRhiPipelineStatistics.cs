using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativePipelineStatistics
{
    public readonly ulong VertexShaderInvocations;

    public readonly ulong RasterizerInputPrimitives;

    public readonly ulong RenderedPrimitives;

    public readonly ulong PixelShaderInvocations;

    public readonly ulong ComputeShaderInvocations;

    internal PipelineStatistics ToPublic()
    {
        return new PipelineStatistics(
            VertexShaderInvocations,
            RasterizerInputPrimitives,
            RenderedPrimitives,
            PixelShaderInvocations,
            ComputeShaderInvocations);
    }
}
