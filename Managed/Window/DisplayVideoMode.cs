#if LUNA_PLATFORM_DESKTOP
using System.Runtime.InteropServices;

namespace Luna.Window;

[StructLayout(LayoutKind.Sequential)]
public readonly struct DisplayVideoMode
{
    public readonly uint Width;

    public readonly uint Height;

    public readonly uint BitsPerPixel;

    public readonly uint RefreshRate;

    public override string ToString()
    {
        return $"{Width}x{Height} {BitsPerPixel}bpp {RefreshRate}Hz";
    }
}
#endif
