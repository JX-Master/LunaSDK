cbuffer vertexBuffer : register (b0)
{
    float4x4 world_to_proj;
};
struct VS_INPUT
{
    [[vk::location(0)]]
    float3 position : POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
PS_INPUT main (VS_INPUT input)
{
    PS_INPUT output;
    output. position = mul (world_to_proj, float4 (input. position, 1.0f));
    output. texcoord = input. texcoord;
    return output;
}