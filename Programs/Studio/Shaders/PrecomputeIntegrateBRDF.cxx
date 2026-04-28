#include "IBLCommon.hxx"
#include <cppsl/texture.hxx>

struct CB
{
    float2 texel_size;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CB cb;

    [[cppsl::binding(1)]]
    RWTexture2D<float4> g_dst_tex;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = dispatch_thread_id.xy;
    float2 texcoords = g_set0.cb.texel_size * (xy(dispatch_thread_id) + 0.5f);
    float3 value = get_integrate_brdf(texcoords.x, texcoords.y);
    g_set0.g_dst_tex.Store(pixel, float4{value, 1.0f});
}
