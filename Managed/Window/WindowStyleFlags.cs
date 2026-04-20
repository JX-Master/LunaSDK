using System;

namespace Luna.Window;

[Flags]
public enum WindowStyleFlags : uint
{
    None = 0,
    Resizable = 0x01,
    Borderless = 0x02
}
