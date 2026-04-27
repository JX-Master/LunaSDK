#include "GeometryDescSet.hxx"

struct MeshVertex
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(1)]] float3 normal;
    [[cppsl::location(2)]] float3 tangent;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] float4 color;
};

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
PS_INPUT vs_main(MeshVertex v)
{
    PS_INPUT o;
    float4 world_position = mul(g_set0.g_MeshBuffer[0].model_to_world, float4{v.position, 1.0f});
    o.world_position = world_position.xyz;
    o.position = mul(g_set0.cb_param.world_to_proj, float4{o.world_position, 1.0f});
    o.normal = mul(float4{v.normal, 0.0f}, g_set0.g_MeshBuffer[0].world_to_model).xyz;
    o.tangent = mul(float4{v.tangent, 0.0f}, g_set0.g_MeshBuffer[0].world_to_model).xyz;
    o.texcoord = v.texcoord;
    o.color = v.color;
    return o;
}
