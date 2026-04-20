using System;

namespace Luna.RHI;

[Flags]
public enum TextureUsageFlags : uint
{
    None = 0x00,
    CopySource = 0x01,
    CopyDestination = 0x02,
    ReadTexture = 0x04,
    ReadWriteTexture = 0x08,
    ColorAttachment = 0x10,
    DepthStencilAttachment = 0x20,
    ResolveAttachment = 0x40,
    Cube = 0x80
}
