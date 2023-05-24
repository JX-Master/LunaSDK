struct MeshVertex
{
    [[vk::location(0)]]
    float3 position : POSITION;
    [[vk::location(1)]]
    float3 normal : NORMAL;
    [[vk::location(2)]]
    float3 tangent : TANGENT;
    [[vk::location(3)]]
    float2 texcoord : TEXCOORD;
    [[vk::location(4)]]
    float4 color : COLOR;
};
struct MeshBuffer
{
	float4x4 model_to_world;	
	float4x4 world_to_model;	
};