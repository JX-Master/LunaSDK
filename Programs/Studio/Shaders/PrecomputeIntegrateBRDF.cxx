#include "IBLCommon.hxx"
#include <cppsl/texture.hxx>

struct CB
{
    float2 texel_size;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
CB cb;

[[cppsl::desc_set(0), cppsl::binding(1)]]
RWTexture2D<float4> g_dst_tex;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float2 texcoords = cb.texel_size * (xy(dispatch_thread_id) + 0.5f);
    float3 value = get_integrate_brdf(texcoords.x, texcoords.y);
    g_dst_tex.Store(pixel, make_float4(value, 1.0f));
}
