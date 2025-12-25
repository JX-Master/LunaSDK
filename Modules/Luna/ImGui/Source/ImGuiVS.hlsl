/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGuiVS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
cbuffer vertexBuffer : register(b0) 
{
    float4x4 ProjectionMatrix; 
};
Texture2D texture0 : register(t1);
SamplerState sampler0 : register(s2);
struct VS_INPUT
{
    [[vk::location(0)]]
    float2 pos : POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
    [[vk::location(2)]]
    float4 col : COLOR0;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
    [[vk::location(2)]]
    float4 col : COLOR0;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}