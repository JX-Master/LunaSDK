/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BlitCommon.hlsl
* @author JXMaster
* @date 2025/11/27
*/

Texture2D<float4> g_src_tex : register(t0);
SamplerState g_sampler : register(s1);

struct VSInput
{
    [[vk::location(0)]]
    float2 position : POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};

struct PSInput
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float2 texcoord : TEXCOORD;
};