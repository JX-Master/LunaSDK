/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BlitPS.hlsl
* @author JXMaster
* @date 2025/11/27
*/
#include "BlitCommon.hlsl"

[[vk::location(0)]]
float4 main(PSInput input) : SV_Target
{
    float4 color = g_src_tex.Sample(g_sampler, input.texcoord);
    return color;
}