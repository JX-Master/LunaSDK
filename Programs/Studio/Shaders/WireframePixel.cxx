#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct CameraCB
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 proj_to_world;
    float4x4 view_to_world;
    float4 env_light_color;
    uint screen_width;
    uint screen_height;
};

struct MeshBuffer
{
    float4x4 model_to_world;
    float4x4 world_to_model;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
CameraCB g_cb;

[[cppsl::structured_buffer, cppsl::desc_set(0), cppsl::binding(1)]]
const MeshBuffer* g_MeshBuffer;

struct PS_INPUT
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float3 normal;
    [[cppsl::location(2)]] float3 tangent;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] float4 color;
    [[cppsl::location(5)]] float3 world_position;
};

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = float4{1.0f, 1.0f, 1.0f, 1.0f};
    return output;
}
