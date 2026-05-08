using Luna.RHI;
using Luna.Runtime;

namespace Luna.ImGui;

public interface ISampledImage : IObject
{
    ITexture? Texture { get; set; }

    SamplerDesc Sampler { get; set; }
}
