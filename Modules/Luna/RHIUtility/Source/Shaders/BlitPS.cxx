#include <cppsl/core.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::desc_set(0), cppsl::binding(0)]]
Texture2D<float4> g_src_tex;

[[cppsl::desc_set(0), cppsl::binding(1)]]
SamplerState g_sampler;

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PSOutput ps_main(PSInput input)
{
    PSOutput output;
    output.color = g_src_tex.Sample(g_sampler, input.texcoord);
    return output;
}
