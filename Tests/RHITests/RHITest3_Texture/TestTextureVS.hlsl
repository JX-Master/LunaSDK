/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TestTextureVS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
struct VS_INPUT
{
    [[vk::location(0)]]
    float2 pos : POSITION;
    [[vk::location(1)]]
    float2 uv : TEXCOORD0;
};
        
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
};
        
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
    output.uv  = input.uv;
    return output;
}