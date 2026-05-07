namespace Luna.VG;

public readonly struct FontAtlasGlyph
{
    public FontAtlasGlyph(ulong firstShapePoint, ulong numShapePoints, RectF boundingRect)
    {
        FirstShapePoint = firstShapePoint;
        NumShapePoints = numShapePoints;
        BoundingRect = boundingRect;
    }

    public ulong FirstShapePoint { get; }
    public ulong NumShapePoints { get; }
    public RectF BoundingRect { get; }
}
