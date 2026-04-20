using System;

namespace Luna.RHI;

public sealed class GraphicsPipelineStateDesc
{
    public InputBindingDesc[] InputBindings { get; init; } = Array.Empty<InputBindingDesc>();

    public InputAttributeDesc[] InputAttributes { get; init; } = Array.Empty<InputAttributeDesc>();

    public IPipelineLayout? PipelineLayout { get; init; }

    public ShaderData VertexShader { get; init; }

    public ShaderData PixelShader { get; init; }

    public RasterizerDesc RasterizerState { get; init; } = RasterizerDesc.Default;

    public DepthStencilDesc DepthStencilState { get; init; } = new();

    public BlendDesc BlendState { get; init; } = new();

    public IndexBufferStripCutValue IndexBufferStripCutValue { get; init; } = IndexBufferStripCutValue.Disabled;

    public PrimitiveTopology PrimitiveTopology { get; init; } = PrimitiveTopology.TriangleList;

    public Format[] ColorFormats { get; init; } = Array.Empty<Format>();

    public Format DepthStencilFormat { get; init; } = Format.Unknown;

    public uint SampleCount { get; init; } = 1;
}
