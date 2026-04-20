using System;
using System.Runtime.InteropServices;

namespace Luna.Runtime;

[Flags]
public enum FileAttributeFlags : uint
{
    None = 0,
    ReadOnly = 0x01,
    Hidden = 0x02,
    Directory = 0x04,
    CharacterSpecial = 0x08,
    BlockSpecial = 0x10
}

[StructLayout(LayoutKind.Sequential)]
public readonly struct FileAttribute
{
    public readonly ulong Size;

    public readonly long CreationTime;

    public readonly long LastAccessTime;

    public readonly long LastWriteTime;

    public readonly FileAttributeFlags Attributes;
}
