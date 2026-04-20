using System;

namespace Luna.RHI;

[Flags]
public enum PipelineLayoutFlags : uint
{
    None = 0,
    AllowInputAssemblerInputLayout = 0x01,
    DenyVertexShaderAccess = 0x02,
    DenyPixelShaderAccess = 0x04
}
