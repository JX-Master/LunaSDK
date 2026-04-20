using System;

namespace Luna.RHI;

[Flags]
public enum ResourceFlags : uint
{
    None = 0,
    AllowAliasing = 0x01
}
