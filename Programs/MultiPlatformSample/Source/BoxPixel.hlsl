Texture2D tex : register (t1);
SamplerState tex_sampler : register (s2);
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
[[vk::location(0)]]
float4 main (PS_INPUT input) : SV_Target
{
    return float4 (tex.Sample (tex_sampler, input. texcoord));
}