using System.Runtime.InteropServices;

namespace Luna.ImGui;

[StructLayout(LayoutKind.Sequential)]
public readonly struct GlyphRange
{
    public GlyphRange(ushort start, ushort end)
    {
        Start = start;
        End = end;
    }

    public ushort Start { get; }

    public ushort End { get; }
}
