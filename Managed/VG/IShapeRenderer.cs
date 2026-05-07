using System.Numerics;
using Luna.RHI;
using Luna.Runtime;

namespace Luna.VG;

public interface IShapeRenderer : IObject
{
    void Begin(ITexture renderTarget);

    void Draw(IBuffer vertexBuffer, IBuffer indexBuffer, ShapeDrawCall[] drawCalls);

    void Draw(IBuffer vertexBuffer, IBuffer indexBuffer, ShapeDrawCall[] drawCalls, Matrix4x4 transformMatrix);

    void End();

    void Submit(ICommandBuffer commandBuffer);
}
