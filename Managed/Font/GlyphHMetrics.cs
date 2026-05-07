using System.Runtime.InteropServices;

namespace Luna.Font;

[StructLayout(LayoutKind.Sequential)]
public readonly struct GlyphHMetrics
{
    public GlyphHMetrics(int advanceWidth, int leftSideBearing)
    {
        AdvanceWidth = advanceWidth;
        LeftSideBearing = leftSideBearing;
    }

    public readonly int AdvanceWidth;

    public readonly int LeftSideBearing;
}
