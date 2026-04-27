#include <cppsl/core.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct DescSet0
{
    [[cppsl::binding(0)]]
    Texture2D<float4> g_src_tex;

    [[cppsl::binding(1)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

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
PSOutput ps_main(PSInput v)
{
    PSOutput o;
    o.color = g_set0.g_src_tex.Sample(g_set0.g_sampler, v.texcoord);
    return o;
}
