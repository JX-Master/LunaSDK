struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};
SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

float4 main(PS_INPUT input) : SV_Target
{
	float4 out_col = clamp(texture0.Sample(sampler0, input.uv), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(1.0f, 1.0f, 1.0f, 1.0f));
	return out_col;
}