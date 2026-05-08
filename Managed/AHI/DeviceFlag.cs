using System;

namespace Luna.AHI;

[Flags]
public enum DeviceFlag : uint
{
    None = 0,
    Playback = 1,
    Capture = 2
}
