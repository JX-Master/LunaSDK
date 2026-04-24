#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct VertexBuffer
{
    float4x4 ProjectionMatrix;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
VertexBuffer vertexBuffer;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float4> texture0;

[[cppsl::desc_set(0), cppsl::binding(2)]]
SamplerState sampler0;

struct PS_INPUT
{
    [[cppsl::position]] float4 pos;
    [[cppsl::location(1)]] float2 uv;
    [[cppsl::location(2)]] float4 col;
};

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = input.col * texture0.Sample(sampler0, input.uv);
    return output;
}
