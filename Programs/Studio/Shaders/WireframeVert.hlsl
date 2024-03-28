#include "CommonVertex.hlsl"
#include "MeshBuffer.hlsl"
cbuffer CBParam : register(b0)
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
StructuredBuffer<MeshBuffer> g_MeshBuffer : register(t1);
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float3 normal   : NORMAL;
    [[vk::location(2)]]
    float3 tangent  : TANGENT;
    [[vk::location(3)]]
    float2 texcoord : TEXCOORD;
    [[vk::location(4)]]
    float4 color    : COLOR;
    [[vk::location(5)]]
    float3 world_position : POSITION;
};

PS_INPUT main(MeshVertex input)
{
    PS_INPUT output;
    output.world_position = mul(g_MeshBuffer[0].model_to_world, float4(input.position, 1.0f)).xyz;
    output.position = mul(world_to_proj, float4(output.world_position, 1.0f));
    output.normal = mul(float4(input.normal, 0.0f), g_MeshBuffer[0].world_to_model).xyz;
    output.tangent = mul(float4(input.tangent, 0.0f), g_MeshBuffer[0].world_to_model).xyz;
    output.texcoord = input.texcoord;    
    output.color = input.color;    
    return output;
}