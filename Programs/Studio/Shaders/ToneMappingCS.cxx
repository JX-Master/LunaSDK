#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct ToneMappingParams
{
    float g_exposure;
    uint g_auto_exposure;
    uint g_dst_width;
    uint g_dst_height;
    float g_bloom_intensity;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
ToneMappingParams tone_mapping_params;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float4> g_scene_tex;

[[cppsl::desc_set(0), cppsl::binding(2)]]
Texture2D<float> g_lum_tex;

[[cppsl::desc_set(0), cppsl::binding(3)]]
Texture2D<float> g_bloom_tex;

[[cppsl::desc_set(0), cppsl::binding(4)]]
RWTexture2D<float4> g_dst_tex;

[[cppsl::desc_set(0), cppsl::binding(5)]]
SamplerState g_sampler;

float3 aces_film(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

float3 tonemap(float3 color, float exposure)
{
    color *= exposure;
    return aces_film(color);
}

float3 gamma_correction(float3 color, float gamma)
{
    return float3{pow(abs(color.x), 1.0f / gamma), pow(abs(color.y), 1.0f / gamma), pow(abs(color.z), 1.0f / gamma)};
}

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float3 hdr_color = xyz(g_scene_tex.Load(pixel));
    float2 texel_size = float2{1.0f / float(tone_mapping_params.g_dst_width), 1.0f / float(tone_mapping_params.g_dst_height)};
    float2 uv = texel_size * (xy(dispatch_thread_id) + 0.5f);
    float bloom_color = g_bloom_tex.SampleLevel(g_sampler, uv, 0.0f) * tone_mapping_params.g_bloom_intensity;
    hdr_color += bloom_color;

    float exposure;
    if (tone_mapping_params.g_auto_exposure > 0u)
    {
        float average_luminance = g_lum_tex.Load(uint2{0u, 0u});
        exposure = tone_mapping_params.g_exposure / max(0.0001f, average_luminance);
    }
    else
    {
        exposure = tone_mapping_params.g_exposure;
    }

    float3 ldr_color = tonemap(hdr_color, exposure);
    float3 final_color = gamma_correction(ldr_color, 2.2f);
    g_dst_tex.Store(pixel, make_float4(saturate(final_color), 1.0f));
}
