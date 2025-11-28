/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BlitVS.hlsl
* @author JXMaster
* @date 2025/11/27
*/
#include "BlitCommon.hlsl"

PSInput main(VSInput input)
{
    PSInput output;
    output.position = float4(input.position.xy, 0.0f, 1.0f);
    output.texcoord = input.texcoord;
    return output;
}