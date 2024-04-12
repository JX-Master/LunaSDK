struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float4 col : COLOR0;
};
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return input.col;
}