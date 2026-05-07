using System.Numerics;
using Luna.RHI;
using Luna.Runtime;

namespace Luna.VG;

public interface IShapeDrawList : IObject
{
    IDevice GetDevice();

    void Reset();

    void SetShapeBuffer(IShapeBuffer? shapeBuffer);

    IShapeBuffer GetShapeBuffer();

    void SetTexture(ITexture? texture);

    ITexture? GetTexture();

    void SetSampler(SamplerDesc? desc);

    SamplerDesc GetSampler();

    void SetTransform(Matrix4x4 transform);

    Matrix4x4 GetTransform();

    void SetClipRect(RectF clipRect);

    RectF GetClipRect();

    void DrawShapeRaw(Vertex[] vertices, uint[] indices);

    void DrawShape(
        uint beginCommand,
        uint numCommands,
        Vector2 minPosition,
        Vector2 maxPosition,
        Vector2 minShapeCoord,
        Vector2 maxShapeCoord,
        Vector4? color = null,
        Vector2? minTexCoord = null,
        Vector2? maxTexCoord = null);

    void Compile();

    IBuffer? GetVertexBuffer();

    uint GetVertexBufferSize();

    IBuffer? GetIndexBuffer();

    uint GetIndexBufferSize();

    ShapeDrawCall[] GetDrawCalls();
}
