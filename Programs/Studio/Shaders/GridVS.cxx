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

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CameraCB vertexBuffer;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VS_INPUT
{
    [[cppsl::location(0)]] float4 pos;
};

struct PS_INPUT
{
    [[cppsl::position]] float4 pos;
};

[[cppsl::vertex]]
PS_INPUT vs_main(VS_INPUT v)
{
    PS_INPUT o;
    o.pos = mul(g_set0.vertexBuffer.world_to_proj, v.pos);
    return o;
}
