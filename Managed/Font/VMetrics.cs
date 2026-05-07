using System.Runtime.InteropServices;

namespace Luna.Font;

[StructLayout(LayoutKind.Sequential)]
public readonly struct VMetrics
{
    public VMetrics(int ascent, int descent, int lineGap)
    {
        Ascent = ascent;
        Descent = descent;
        LineGap = lineGap;
    }

    public readonly int Ascent;

    public readonly int Descent;

    public readonly int LineGap;
}
