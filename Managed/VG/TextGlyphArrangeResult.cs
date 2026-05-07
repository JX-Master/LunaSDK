namespace Luna.VG;

public readonly struct TextGlyphArrangeResult
{
    public TextGlyphArrangeResult(RectF boundingRect, float originOffset, float advanceLength, uint character, uint index)
    {
        BoundingRect = boundingRect;
        OriginOffset = originOffset;
        AdvanceLength = advanceLength;
        Character = character;
        Index = index;
    }

    public RectF BoundingRect { get; }
    public float OriginOffset { get; }
    public float AdvanceLength { get; }
    public uint Character { get; }
    public uint Index { get; }
}
