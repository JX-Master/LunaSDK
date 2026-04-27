#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct CB
{
    uint dst_tex_width;
    uint dst_tex_height;
    float lum_threshold;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CB cb;

    [[cppsl::binding(1)]]
    Texture2D<float4> g_src_tex;

    [[cppsl::binding(2)]]
    RWTexture2D<float4> g_dst_tex;

    [[cppsl::binding(3)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

float4 setup_sample(float2 uv)
{
    float3 color = g_set0.g_src_tex.SampleLevel(g_set0.g_sampler, uv, 0.0f).xyz;
    float lum = luminance(color);
    if (lum < g_set0.cb.lum_threshold)
    {
        return float4{0.0f, 0.0f, 0.0f, 1.0f};
    }
    color *= (lum - g_set0.cb.lum_threshold) / lum;
    lum = luminance(color);
    float w = 1.0f / (1.0f + lum);
    return make_float4(color, 1.0f) * w;
}

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = dispatch_thread_id.xy;
    float4 color = float4{0.0f, 0.0f, 0.0f, 0.0f};
    float2 texel_size = float2{1.0f / float(g_set0.cb.dst_tex_width), 1.0f / float(g_set0.cb.dst_tex_height)};
    float2 uv = texel_size * (xy(dispatch_thread_id) + 0.5f);
    float2 stride = texel_size / 2.0f;
    color += setup_sample(uv + stride) * 0.125f;
    color += setup_sample(uv - stride) * 0.125f;
    color += setup_sample(uv + float2{stride.x, -stride.y}) * 0.125f;
    color += setup_sample(uv + float2{-stride.x, stride.y}) * 0.125f;
    color += setup_sample(uv) * 0.5f;
    color /= color.w;
    g_set0.g_dst_tex.Store(pixel, color);
}
