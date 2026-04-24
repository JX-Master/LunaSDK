#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct CameraCB
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 view_to_world;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
CameraCB vertexBuffer;

struct VS_INPUT
{
    [[cppsl::location(0)]] float4 pos;
};

struct PS_INPUT
{
    [[cppsl::position]] float4 pos;
};

[[cppsl::vertex]]
PS_INPUT vs_main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(vertexBuffer.world_to_proj, input.pos);
    return output;
}
