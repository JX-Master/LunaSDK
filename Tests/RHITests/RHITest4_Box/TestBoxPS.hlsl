/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TestBoxPS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
cbuffer vertexBuffer : register(b0)
{
    float4x4 world_to_proj;
};
Texture2D tex : register(t1);
SamplerState tex_sampler : register(s2);
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return float4(tex.Sample(tex_sampler, input.texcoord));
}