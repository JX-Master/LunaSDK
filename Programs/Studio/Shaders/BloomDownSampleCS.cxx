#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct CB
{
    uint dst_tex_width;
    uint dst_tex_height;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
CB cb;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float4> g_src_tex;

[[cppsl::desc_set(0), cppsl::binding(2)]]
RWTexture2D<float4> g_dst_tex;

[[cppsl::desc_set(0), cppsl::binding(3)]]
SamplerState g_sampler;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float3 color = float3{0.0f, 0.0f, 0.0f};
    float2 texel_size = float2{1.0f / float(cb.dst_tex_width), 1.0f / float(cb.dst_tex_height)};
    float2 uv = texel_size * (xy(dispatch_thread_id) + 0.5f);
    float2 stride = texel_size / 2.0f;
    color += xyz(g_src_tex.SampleLevel(g_sampler, uv + stride, 0.0f)) * 0.125f;
    color += xyz(g_src_tex.SampleLevel(g_sampler, uv - stride, 0.0f)) * 0.125f;
    color += xyz(g_src_tex.SampleLevel(g_sampler, uv + float2{stride.x, -stride.y}, 0.0f)) * 0.125f;
    color += xyz(g_src_tex.SampleLevel(g_sampler, uv + float2{-stride.x, stride.y}, 0.0f)) * 0.125f;
    color += xyz(g_src_tex.SampleLevel(g_sampler, uv, 0.0f)) * 0.5f;
    g_dst_tex.Store(pixel, make_float4(color, 1.0f));
}
