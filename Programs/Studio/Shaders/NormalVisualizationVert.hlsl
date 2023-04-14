#include "CommonVertex.hlsl"

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
struct GS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

GS_INPUT main(MeshVertex input)
{
	GS_INPUT output;
	output.position = mul(g_MeshBuffer[0].model_to_world, float4(input.position, 1.0f)).xyz;
	output.normal = normalize(mul(float4(input.normal, 0.0f), g_MeshBuffer[0].world_to_model).xyz);
	output.tangent = normalize(mul(float4(input.tangent, 0.0f), g_MeshBuffer[0].world_to_model).xyz);
	return output;
}