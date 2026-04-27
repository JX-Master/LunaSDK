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
PSInput vs_main(VSInput v)
{
    PSInput o;
    o.position = float4{v.position, 0.0f, 1.0f};
    o.texcoord = v.texcoord;
    return o;
}
