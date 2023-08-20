#include "DepthCommon.hlsl"
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};

void main(PS_INPUT i)
{
    float2 texcoord = float2(i.texcoord.x, 1.0f - i.texcoord.y);
    float4 base_color = g_base_color.Sample(g_sampler, texcoord);
    clip(base_color.w - 0.1f);
}