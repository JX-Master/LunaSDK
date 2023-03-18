cbuffer vertex_buffer : register(b0)
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 view_to_world;
};

struct MeshBuffer
{
    float4x4 model_to_world;
    float4x4 world_to_model;
};

StructuredBuffer<MeshBuffer> g_MeshBuffer : register(t1);

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

float4 main(VS_INPUT input) : SV_POSITION
{
    float4 world_position = mul(g_MeshBuffer[0].model_to_world, float4(input.position, 1.0f));
    return mul(world_to_proj, world_position);
}