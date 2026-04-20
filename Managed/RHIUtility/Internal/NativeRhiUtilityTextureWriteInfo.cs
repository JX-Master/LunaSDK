using System.Runtime.InteropServices;

namespace Luna.RHIUtility.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiUtilityTextureWriteInfo
{
    public readonly uint RowPitch;
    public readonly uint SlicePitch;

    internal TextureWriteInfo ToPublic()
    {
        return new TextureWriteInfo(RowPitch, SlicePitch);
    }
}
