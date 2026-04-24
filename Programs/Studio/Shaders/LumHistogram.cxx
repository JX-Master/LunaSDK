#include "StudioCommon.hxx"
#include <cppsl/compute.hxx>
#include <cppsl/texture.hxx>

struct LumHistogramCB
{
    uint src_width;
    uint src_height;
    float min_brightness;
    float max_brightness;
    float bloom_intensity;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
LumHistogramCB cb;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float4> g_src_tex;

[[cppsl::desc_set(0), cppsl::binding(2)]]
Texture2D<float4> g_bloom_tex;

[[cppsl::rwstructured_buffer, cppsl::desc_set(0), cppsl::binding(3)]]
uint* g_dst_buffer;

[[cppsl::desc_set(0), cppsl::binding(4)]]
SamplerState g_sampler;

[[cppsl::group_shared]]
uint histogram_shared[256];

uint color_to_bin(float3 hdr_color, float min_brightness, float max_brightness)
{
    float lum = dot(hdr_color, float3{0.2125f, 0.7154f, 0.0721f});
    if (lum < min_brightness)
    {
        return 0u;
    }

    float min_log_lum = log2(min_brightness);
    float log_lum_range = log2(max_brightness) - min_log_lum;
    float log_lum = clamp((log2(lum) - min_log_lum) / log_lum_range, 0.0f, 1.0f);
    return uint(log_lum * 254.0f + 1.0f);
}

[[cppsl::compute(16, 16, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id, [[cppsl::builtin(group_index)]] uint group_index)
{
    histogram_shared[group_index] = 0u;
    GroupMemoryBarrierWithGroupSync();

    if (dispatch_thread_id.x < cb.src_width && dispatch_thread_id.y < cb.src_height)
    {
        uint2 pixel = xy_u(dispatch_thread_id);
        float3 hdr_color = xyz(g_src_tex.Load(pixel));
        float2 texel_size = float2{2.0f / float(cb.src_width), 2.0f / float(cb.src_height)};
        float2 uv = texel_size * (xy(dispatch_thread_id) + 0.5f);
        float3 bloom_color = xyz(g_bloom_tex.SampleLevel(g_sampler, uv, 0.0f)) * cb.bloom_intensity;
        hdr_color += bloom_color;
        uint bin_index = color_to_bin(hdr_color, cb.min_brightness, cb.max_brightness);
        InterlockedAdd(histogram_shared[bin_index], 1u);
    }

    GroupMemoryBarrierWithGroupSync();
    InterlockedAdd(g_dst_buffer[group_index], histogram_shared[group_index]);
}
