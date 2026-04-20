using System;

namespace Luna.RHI;

public sealed class BlendDesc
{
    public bool AlphaToCoverageEnable { get; init; }

    public bool IndependentBlendEnable { get; init; }

    public AttachmentBlendDesc[] Attachments { get; init; } = Array.Empty<AttachmentBlendDesc>();
}
