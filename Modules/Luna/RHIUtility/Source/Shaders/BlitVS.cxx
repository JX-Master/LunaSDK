#include <cppsl/core.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::desc_set(0), cppsl::binding(0)]]
Texture2D<float4> g_src_tex;

[[cppsl::desc_set(0), cppsl::binding(1)]]
SamplerState g_sampler;

struct VSInput
{
    [[cppsl::location(0)]] float2 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 texcoord;
};

[[cppsl::vertex]]
PSInput vs_main(VSInput input)
{
    PSInput output;
    output.position = float4{input.position.x, input.position.y, 0.0f, 1.0f};
    output.texcoord = input.texcoord;
    return output;
}
