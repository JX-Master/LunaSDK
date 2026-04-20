using System.Runtime.InteropServices;

namespace Luna.Image.Internal;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeImageDesc
{
    public uint Format;
    public uint Width;
    public uint Height;

    public readonly ImageDesc ToManaged()
    {
        return new ImageDesc((ImageFormat)Format, Width, Height);
    }
}
