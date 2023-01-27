struct VS_INPUT
{
	float2 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
	output.uv = input.uv;
	return output;
}