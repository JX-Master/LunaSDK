struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float3 normal : NORMAL;
    [[vk::location(2)]]
    float3 tangent : TANGENT;
    [[vk::location(3)]]
    float2 texcoord : TEXCOORD;
    [[vk::location(4)]]
    float4 color : COLOR;
    [[vk::location(5)]]
    float3 world_position : POSITION;
};

float4 main(PS_INPUT input) : SV_Target
{
  return float4(1.0f, 1.0f, 1.0f, 1.0f);
}