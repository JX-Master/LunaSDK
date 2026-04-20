using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeBufferDesc
{
    public readonly ulong Size;
    public readonly uint Usages;
    public readonly uint Flags;

    private NativeBufferDesc(ulong size, uint usages, uint flags)
    {
        Size = size;
        Usages = usages;
        Flags = flags;
    }

    internal static NativeBufferDesc FromPublic(BufferDesc desc)
    {
        return new NativeBufferDesc(desc.Size, (uint)desc.Usages, (uint)desc.Flags);
    }

    internal BufferDesc ToPublic()
    {
        return new BufferDesc((BufferUsageFlags)Usages, Size, (ResourceFlags)Flags);
    }
}
