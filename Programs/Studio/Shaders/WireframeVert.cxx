#include "WireframeDescSet.hxx"

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
    float4 world_position4;
    world_position4 = mul(g_set0.g_MeshBuffer[0].model_to_world, float4{v.position, 1.0f});
    o.world_position = world_position4.xyz;
    o.position = mul(g_set0.g_cb.world_to_proj, float4{o.world_position, 1.0f});

    float4 normal4;
    normal4 = mul(float4{v.normal, 0.0f}, g_set0.g_MeshBuffer[0].world_to_model);
    o.normal = normal4.xyz;

    float4 tangent4;
    tangent4 = mul(float4{v.tangent, 0.0f}, g_set0.g_MeshBuffer[0].world_to_model);
    o.tangent = tangent4.xyz;

    o.texcoord = v.texcoord;
    o.color = v.color;
    return o;
}
