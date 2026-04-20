using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeCommandQueueDesc
{
    public readonly uint Type;

    public readonly uint Flags;
}
