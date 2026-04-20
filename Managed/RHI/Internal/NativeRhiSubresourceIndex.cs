using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeSubresourceIndex
{
    public readonly uint MipSlice;
    public readonly uint ArraySlice;

    internal NativeSubresourceIndex(SubresourceIndex index)
    {
        MipSlice = index.MipSlice;
        ArraySlice = index.ArraySlice;
    }
}
