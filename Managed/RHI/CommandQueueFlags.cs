using System;

namespace Luna.RHI;

[Flags]
public enum CommandQueueFlags : uint
{
    None = 0,
    Presenting = 0x01
}
