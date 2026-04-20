using System;

namespace Luna.RHI;

[Flags]
public enum BufferUsageFlags : uint
{
    None = 0x00,
    CopySource = 0x01,
    CopyDestination = 0x02,
    UniformBuffer = 0x04,
    ReadBuffer = 0x08,
    ReadWriteBuffer = 0x10,
    VertexBuffer = 0x20,
    IndexBuffer = 0x40,
    IndirectBuffer = 0x80
}
