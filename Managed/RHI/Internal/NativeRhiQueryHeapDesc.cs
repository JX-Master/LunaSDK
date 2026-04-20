using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeQueryHeapDesc
{
    public readonly uint Type;

    public readonly uint Count;

    internal NativeQueryHeapDesc(QueryHeapDesc desc)
    {
        Type = (uint)desc.Type;
        Count = desc.Count;
    }

    internal QueryHeapDesc ToPublic()
    {
        return new QueryHeapDesc((QueryType)Type, Count);
    }
}
