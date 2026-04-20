using System;

namespace Luna.Window;

[Flags]
public enum FileDialogFlags : uint
{
    None = 0,
    MultiSelect = 0x01,
    AnyFile = 0x02
}
