using System.Numerics;
using Luna.RHI;

namespace Luna.VG;

public readonly struct ShapeDrawCall
{
    public ShapeDrawCall(IBuffer? shapeBuffer, ITexture? texture, SamplerDesc sampler, RectF clipRect, uint baseIndex, uint numIndices, Matrix4x4 transform)
    {
        ShapeBuffer = shapeBuffer;
        Texture = texture;
        Sampler = sampler;
        ClipRect = clipRect;
        BaseIndex = baseIndex;
        NumIndices = numIndices;
        Transform = transform;
    }

    public IBuffer? ShapeBuffer { get; }
    public ITexture? Texture { get; }
    public SamplerDesc Sampler { get; }
    public RectF ClipRect { get; }
    public uint BaseIndex { get; }
    public uint NumIndices { get; }
    public Matrix4x4 Transform { get; }
}
