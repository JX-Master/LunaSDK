/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TestTexturePS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
};
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s1);

[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    float4 out_col = clamp(texture0.Sample(sampler0, input.uv), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)); 
    return out_col; 
}