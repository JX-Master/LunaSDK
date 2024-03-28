#include "IBLCommon.hlsl"

cbuffer CB : register(b0)
{
    float2 texel_size;    // 1.0 / destination dimension
}
RWTexture2D<float4> g_dst_tex : register(u1);

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
    float2 texcoords = texel_size * (dispatch_thread_id.xy + 0.5f);
    float3 value = get_integrate_brdf(texcoords.x, texcoords.y);
    g_dst_tex[dispatch_thread_id.xy] = float4(value, 1.0f);
}