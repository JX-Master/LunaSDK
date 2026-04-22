#include <cppsl/core.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

[[cppsl::cbuffer, cppsl::set(0), cppsl::binding(0)]]
Camera vertexBuffer;

[[cppsl::set(0), cppsl::binding(1)]]
Texture2D<float4> tex;

[[cppsl::set(0), cppsl::binding(2)]]
SamplerState tex_sampler;

struct PS_INPUT
{
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
    output.color = tex.Sample(tex_sampler, input.texcoord);
    return output;
}
