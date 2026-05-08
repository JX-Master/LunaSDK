using System;

namespace Luna.HID;

[Flags]
public enum MouseButton : byte
{
    None = 0x00,
    Left = 0x01,
    Right = 0x02,
    Middle = 0x04,
    Function1 = 0x08,
    Function2 = 0x10
}
