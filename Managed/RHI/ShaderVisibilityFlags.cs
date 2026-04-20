using System;

namespace Luna.RHI;

[Flags]
public enum ShaderVisibilityFlags : uint
{
    None = 0x00,
    Vertex = 0x01,
    Pixel = 0x02,
    Compute = 0x04,
    All = Vertex | Pixel | Compute
}
