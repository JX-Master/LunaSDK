using System;

namespace Luna.HID;

[Flags]
public enum ControllerButton : uint
{
    None = 0x0000,
    LThumb = 0x0001,
    RThumb = 0x0002,
    Up = 0x0004,
    Down = 0x0008,
    Left = 0x0010,
    Right = 0x0020,
    A = 0x0040,
    B = 0x0080,
    X = 0x0100,
    Y = 0x0200,
    Lb = 0x0400,
    Rb = 0x0800,
    Lt = 0x1000,
    Rt = 0x2000,
    LSpecial = 0x4000,
    RSpecial = 0x8000
}
