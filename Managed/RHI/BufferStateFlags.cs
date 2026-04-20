using System;

namespace Luna.RHI;

[Flags]
public enum BufferStateFlags : uint
{
    None = 0x00,
    IndirectArgument = 0x01,
    VertexBuffer = 0x02,
    IndexBuffer = 0x04,
    UniformBufferVs = 0x08,
    ShaderReadVs = 0x10,
    UniformBufferPs = 0x20,
    ShaderReadPs = 0x40,
    ShaderWritePs = 0x80,
    UniformBufferCs = 0x0100,
    ShaderReadCs = 0x0200,
    ShaderWriteCs = 0x0400,
    CopyDestination = 0x0800,
    CopySource = 0x1000,
    Automatic = 0x80000000,
    ShaderReadWritePs = ShaderReadPs | ShaderWritePs,
    ShaderReadWriteCs = ShaderReadCs | ShaderWriteCs
}
