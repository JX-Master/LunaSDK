/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGuiPS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
    [[vk::location(2)]]
    float4 col : COLOR0;
};
cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};
Texture2D texture0 : register(t1);
SamplerState sampler0 : register(s2);
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv); 
    return out_col; 
}