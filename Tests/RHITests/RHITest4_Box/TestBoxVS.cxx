#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/resource.hxx>
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
    output.position = mul(vertexBuffer.world_to_proj, float4{input.position.x, input.position.y, input.position.z, 1.0f});
    output.texcoord = input.texcoord;
    return output;
}
