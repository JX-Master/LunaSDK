#include "StudioCommon.hxx"

struct MeshVertex
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(1)]] float3 normal;
    [[cppsl::location(2)]] float3 tangent;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] float4 color;
};

struct MeshBuffer
{
    float4x4 model_to_world;
    float4x4 world_to_model;
};

struct CBParam
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

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
CBParam cb_param;

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

[[cppsl::vertex]]
PS_INPUT vs_main(MeshVertex input)
{
    PS_INPUT output;
    float4 world_position = mul(g_MeshBuffer[0].model_to_world, float4{input.position.x, input.position.y, input.position.z, 1.0f});
    output.world_position = xyz(world_position);
    output.position = mul(cb_param.world_to_proj, float4{output.world_position.x, output.world_position.y, output.world_position.z, 1.0f});
    output.normal = xyz(mul(float4{input.normal.x, input.normal.y, input.normal.z, 0.0f}, g_MeshBuffer[0].world_to_model));
    output.tangent = xyz(mul(float4{input.tangent.x, input.tangent.y, input.tangent.z, 0.0f}, g_MeshBuffer[0].world_to_model));
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}
