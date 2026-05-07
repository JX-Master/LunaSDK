using Luna.Font;
using Luna.Runtime;

namespace Luna.VG;

public interface IFontAtlas : IObject
{
    void Clear();

    IFontFile? GetFont(out uint index);

    void SetFont(IFontFile fontFile, uint index);

    IShapeBuffer GetShapeBuffer();

    FontAtlasGlyph GetGlyph(uint codepoint);
}
