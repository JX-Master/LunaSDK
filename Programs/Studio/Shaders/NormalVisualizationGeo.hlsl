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

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

[maxvertexcount(6)]
void main(point GS_INPUT input[1], inout LineStream<PS_INPUT> output)
{
    PS_INPUT origin;
    PS_INPUT dir;

    float3 normal = normalize(input[0].normal);
    float3 tangent = normalize(input[0].tangent);
    float3 binormal = normalize(cross(normal, tangent));

    // Draw normal.
    origin.position = mul(world_to_proj, float4(input[0].position, 1.0f));
    origin.color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    output.Append(origin);
    float3 dir_position = input[0].position + normal * 0.25f;
    dir.position = mul(world_to_proj, float4(dir_position, 1.0f));
    dir.color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    output.Append(dir);

    // Draw tangent.
    output.RestartStrip();
    origin.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.Append(origin);
    dir_position = input[0].position + tangent * 0.25f;
    dir.position = mul(world_to_proj, float4(dir_position, 1.0f));
    dir.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.Append(dir);

    // Draw binormal.
    output.RestartStrip();
    origin.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    output.Append(origin);
    dir_position = input[0].position + binormal * 0.25f;
    dir.position = mul(world_to_proj, float4(dir_position, 1.0f));
    dir.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    output.Append(dir);
}