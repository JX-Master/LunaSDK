struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
};
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}