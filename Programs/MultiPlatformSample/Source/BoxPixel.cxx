#include <cppsl/core.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct VertexBuffer
{
    float4x4 world_to_proj;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    VertexBuffer vertexBuffer;

    [[cppsl::binding(1)]]
    Texture2D<float4> tex;

    [[cppsl::binding(2)]]
    SamplerState tex_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct PS_INPUT
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = g_set0.tex.Sample(g_set0.tex_sampler, input.texcoord);
    return output;
}
