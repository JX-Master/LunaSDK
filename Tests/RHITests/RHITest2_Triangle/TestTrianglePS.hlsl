/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TestTrianglePS.hlsl
* @author JXMaster
* @date 2024/4/12
*/
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float4 col : COLOR0;
};
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    return input.col;
}