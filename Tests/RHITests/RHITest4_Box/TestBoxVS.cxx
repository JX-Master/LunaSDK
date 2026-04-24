#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/resource.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

struct DescSet0
{
    [[cppsl::binding(15)]]
    SamplerState tex_sampler;

    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera vertexBuffer;

    [[cppsl::binding(8)]]
    Texture2D<float4> tex;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VS_INPUT
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct PS_INPUT
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 texcoord;
};

[[cppsl::vertex]]
PS_INPUT vs_main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(g_set0.vertexBuffer.world_to_proj, float4{input.position.x, input.position.y, input.position.z, 1.0f});
    output.texcoord = input.texcoord;
    return output;
}
