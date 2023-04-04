struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Texture2D g_base_color : register(t2);
SamplerState g_sampler : register(s3);

void main(PS_INPUT i)
{
    float2 texcoord = float2(i.texcoord.x, 1.0f - i.texcoord.y);
    float4 base_color = g_base_color.Sample(g_sampler, texcoord);
    clip(base_color.w - 0.1f);
}