using System.Runtime.InteropServices;

namespace Luna.RHI.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeTextureDataPlacementInfo
{
    public readonly ulong Size;
    public readonly ulong Alignment;
    public readonly ulong RowPitch;
    public readonly ulong SlicePitch;

    internal TextureDataPlacementInfo ToPublic()
    {
        return new TextureDataPlacementInfo(Size, Alignment, RowPitch, SlicePitch);
    }
}
