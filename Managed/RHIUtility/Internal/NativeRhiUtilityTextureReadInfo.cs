using System.Runtime.InteropServices;

namespace Luna.RHIUtility.Internal;

[StructLayout(LayoutKind.Sequential)]
internal readonly struct NativeRhiUtilityTextureReadInfo
{
    public readonly uint RowPitch;

    public readonly uint SlicePitch;
}
