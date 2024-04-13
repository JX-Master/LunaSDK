/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TestBoxVS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
cbuffer vertexBuffer : register(b0)
{
    float4x4 world_to_proj;
};
Texture2D tex : register(t1);
SamplerState tex_sampler : register(s2);
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
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(world_to_proj, float4(input.position, 1.0f));
    output.texcoord = input.texcoord;
    return output;
}