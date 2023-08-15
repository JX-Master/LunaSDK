#include "CommonVertex.hlsl"
cbuffer vertex_buffer : register(b0)
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 view_to_world;
};
StructuredBuffer<MeshBuffer> g_MeshBuffer : register(t1);
Texture2D g_base_color : register(t2);
SamplerState g_sampler : register(s3);
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};

PS_INPUT main(MeshVertex input)
{
    PS_INPUT o;
    float4 world_position = mul(g_MeshBuffer[0].model_to_world, float4(input.position, 1.0f));
    o.position = mul(world_to_proj, world_position);
    o.texcoord = input.texcoord;
    return o;
}