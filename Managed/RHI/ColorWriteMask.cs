using System;

namespace Luna.RHI;

[Flags]
public enum ColorWriteMask : uint
{
    None = 0,
    Red = 1,
    Green = 2,
    Blue = 4,
    Alpha = 8,
    All = Red | Green | Blue | Alpha
}
