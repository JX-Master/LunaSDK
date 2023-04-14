struct MeshVertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};
struct MeshBuffer
{
	float4x4 model_to_world;	
	float4x4 world_to_model;	
};