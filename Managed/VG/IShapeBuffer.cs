using Luna.RHI;
using Luna.Runtime;

namespace Luna.VG;

public interface IShapeBuffer : IObject
{
    float[] GetShapePoints();

    void SetShapePoints(float[] points);

    IBuffer? Build(IDevice device);
}
