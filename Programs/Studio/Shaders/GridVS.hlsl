cbuffer vertexBuffer : register(b0)
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 view_to_world;
};
struct VS_INPUT
{
  [[vk::location(0)]]
  float4 pos : POSITION;
};

struct PS_INPUT
{
  [[vk::location(0)]]
  float4 pos : SV_POSITION;
};

PS_INPUT main(VS_INPUT input)
{
  PS_INPUT output;
  output.pos = mul(world_to_proj, input.pos);
  return output;
}