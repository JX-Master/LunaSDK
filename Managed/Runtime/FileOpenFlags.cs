using System;

namespace Luna.Runtime;

[Flags]
public enum FileOpenFlags : uint
{
    None = 0,
    Read = 0x01,
    Write = 0x02,
    UserBuffering = 0x04
}
