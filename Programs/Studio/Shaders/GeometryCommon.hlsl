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
Texture2D g_base_color : register(t2);
Texture2D g_roughness : register(t3);
Texture2D g_normal : register(t4);
Texture2D g_metallic : register(t5);
Texture2D g_emissive : register(t6);
SamplerState g_sampler : register(s7);