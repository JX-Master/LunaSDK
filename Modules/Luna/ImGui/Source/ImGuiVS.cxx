#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct VertexBuffer
{
    float4x4 ProjectionMatrix;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    VertexBuffer vertexBuffer;

    [[cppsl::binding(1)]]
    Texture2D<float4> texture0;

    [[cppsl::binding(2)]]
    SamplerState sampler0;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VS_INPUT
{
    [[cppsl::location(0)]] float2 pos;
    [[cppsl::location(1)]] float2 uv;
    [[cppsl::location(2)]] float4 col;
};

struct PS_INPUT
{
    [[cppsl::position]] float4 pos;
    [[cppsl::location(1)]] float2 uv;
    [[cppsl::location(2)]] float4 col;
};

[[cppsl::vertex]]
PS_INPUT vs_main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(g_set0.vertexBuffer.ProjectionMatrix, float4{input.pos.x, input.pos.y, 0.0f, 1.0f});
    output.col = input.col;
    output.uv = input.uv;
    return output;
}
