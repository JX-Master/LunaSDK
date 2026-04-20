using System;

namespace Luna.RHI;

public sealed class RenderPassDesc
{
    public ColorAttachment[] ColorAttachments { get; init; } = Array.Empty<ColorAttachment>();

    public ResolveAttachment[] ResolveAttachments { get; init; } = Array.Empty<ResolveAttachment>();

    public DepthStencilAttachment? DepthStencilAttachment { get; init; }

    public IQueryHeap? OcclusionQueryHeap { get; init; }

    public IQueryHeap? TimestampQueryHeap { get; init; }

    public IQueryHeap? PipelineStatisticsQueryHeap { get; init; }

    public uint TimestampQueryBeginPassWriteIndex { get; init; } = uint.MaxValue;

    public uint TimestampQueryEndPassWriteIndex { get; init; } = uint.MaxValue;

    public uint PipelineStatisticsQueryWriteIndex { get; init; } = uint.MaxValue;

    public uint ArraySize { get; init; } = 1;

    public byte SampleCount { get; init; } = 1;
}
