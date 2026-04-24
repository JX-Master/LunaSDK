#include "BRDF.hxx"
#include <cppsl/texture.hxx>

struct CB
{
    uint tex_width;
    uint tex_height;
    uint mip_0_width;
    uint mip_0_height;
    float roughness;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CB cb;

    [[cppsl::binding(1)]]
    Texture2D<float4> g_src_mip;

    [[cppsl::binding(2)]]
    RWTexture2D<float4> g_dst_mip;

    [[cppsl::binding(3)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    const float PI = 3.1415926f;
    uint2 pixel = xy_u(dispatch_thread_id);
    float2 texcoords = float2{1.0f / float(g_set0.cb.tex_width), 1.0f / float(g_set0.cb.tex_height)} * (xy(dispatch_thread_id) + 0.5f);
    float3 normal = get_dir_from_latlong(texcoords);

    uint num_samples = uint(8192.0f * g_set0.cb.roughness * g_set0.cb.roughness);
    float pixels_per_omega = (float(g_set0.cb.mip_0_width) * float(g_set0.cb.mip_0_height)) / (4.0f * PI);

    float3 prefilter_color = float3{0.0f, 0.0f, 0.0f};
    float total_weight = 0.0f;

    for (uint i = 0u; i < num_samples; ++i)
    {
        float2 xi = hammersley2d(i, num_samples);
        float3 h = importance_sample_ggx(xi, normal, g_set0.cb.roughness);
        float3 l = normalize(2.0f * dot(normal, h) * h - normal);
        float n_dot_l = dot(normal, l);
        if (n_dot_l > 0.0f)
        {
            float n_dot_h = max(1e-8f, dot(normal, h));
            float pdf = ggx_normal_distrb(n_dot_h, g_set0.cb.roughness);
            float sample_omega = 2.0f * PI / (pdf * float(num_samples));
            float sample_pixels = pixels_per_omega * sample_omega;
            float mip_level = 0.5f * log2(sample_pixels);
            float3 radiance = xyz(g_set0.g_src_mip.SampleLevel(g_set0.g_sampler, get_latlong_from_dir(l), mip_level));
            prefilter_color += radiance * n_dot_l;
            total_weight += n_dot_l;
        }
    }

    prefilter_color /= total_weight;
    g_set0.g_dst_mip.Store(pixel, make_float4(prefilter_color, 1.0f));
}
