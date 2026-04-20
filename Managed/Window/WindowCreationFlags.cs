using System;

namespace Luna.Window;

[Flags]
public enum WindowCreationFlags : uint
{
    None = 0,
    Hidden = 0x01
}
