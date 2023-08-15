#include "MeshBuffer.hlsl"
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