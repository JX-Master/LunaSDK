using System;

namespace Luna.RHI;

[Flags]
public enum ResourceBarrierFlags : uint
{
    None = 0,
    Aliasing = 0x01,
    DiscardContent = 0x02
}
