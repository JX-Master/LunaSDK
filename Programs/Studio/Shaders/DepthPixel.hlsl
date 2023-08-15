struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};

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

void main(PS_INPUT i)
{
    float2 texcoord = float2(i.texcoord.x, 1.0f - i.texcoord.y);
    float4 base_color = g_base_color.Sample(g_sampler, texcoord);
    clip(base_color.w - 0.1f);
}