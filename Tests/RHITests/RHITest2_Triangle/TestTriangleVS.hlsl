struct VS_INPUT
{
    [[vk::location(0)]]
    float2 pos : POSITION;
    [[vk::location(1)]]
    float4 col : COLOR0;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float4 col  : COLOR0;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
    output.col  = input.col;
    return output;
}