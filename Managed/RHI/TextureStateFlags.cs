using System;

namespace Luna.RHI;

[Flags]
public enum TextureStateFlags : uint
{
    None = 0x00,
    ShaderReadVs = 0x01,
    ShaderReadPs = 0x02,
    ShaderWritePs = 0x04,
    ColorAttachmentRead = 0x08,
    ColorAttachmentWrite = 0x10,
    DepthStencilAttachmentRead = 0x20,
    DepthStencilAttachmentWrite = 0x40,
    ResolveAttachment = 0x80,
    ShaderReadCs = 0x0100,
    ShaderWriteCs = 0x0200,
    CopyDestination = 0x0400,
    CopySource = 0x0800,
    Present = 0x1000,
    Automatic = 0x80000000,
    ShaderReadWritePs = ShaderReadPs | ShaderWritePs,
    ShaderReadWriteCs = ShaderReadCs | ShaderWriteCs
}
