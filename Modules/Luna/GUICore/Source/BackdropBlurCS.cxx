#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct BackdropBlurParams
{
    uint dst_width;
    uint dst_height;
    float2 source_uv_origin;
    float2 source_uv_size;
    float2 sample_step;
    float gaussian_center_weight;
    uint gaussian_pair_count;
    uint filter_mode;
    uint padding;
    float4 gaussian_pair_0;
    float4 gaussian_pair_1;
    float4 gaussian_pair_2;
    float4 gaussian_pair_3;
    float4 gaussian_pair_4;
    float4 gaussian_pair_5;
    float4 gaussian_pair_6;
    float4 gaussian_pair_7;
    float4 gaussian_pair_8;
    float4 gaussian_pair_9;
    float4 gaussian_pair_10;
    float4 gaussian_pair_11;
    float4 gaussian_pair_12;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    BackdropBlurParams params;

    [[cppsl::binding(1)]]
    Texture2D<float4> source;

    [[cppsl::binding(2)]]
    RWTexture2D<float4> destination;

    [[cppsl::binding(3)]]
    SamplerState sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

float4 sample_source(float2 uv)
{
    return g_set0.source.SampleLevel(g_set0.sampler, uv, 0.0f);
}

float4 downsample_tent(float2 uv)
{
    float2 stride = g_set0.params.source_uv_size /
        float2{float(g_set0.params.dst_width), float(g_set0.params.dst_height)} * 0.5f;
    float4 color = sample_source(uv) * 0.5f;
    color += sample_source(uv + stride) * 0.125f;
    color += sample_source(uv - stride) * 0.125f;
    color += sample_source(uv + float2{stride.x, -stride.y}) * 0.125f;
    color += sample_source(uv + float2{-stride.x, stride.y}) * 0.125f;
    return color;
}

float4 gaussian_pair(float2 uv, float4 pair)
{
    float2 offset = g_set0.params.sample_step * pair.x;
    return (sample_source(uv + offset) + sample_source(uv - offset)) * pair.y;
}

float4 gaussian_filter(float2 uv)
{
    float4 color = sample_source(uv) * g_set0.params.gaussian_center_weight;
    if(g_set0.params.gaussian_pair_count > 0u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_0);
    if(g_set0.params.gaussian_pair_count > 1u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_1);
    if(g_set0.params.gaussian_pair_count > 2u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_2);
    if(g_set0.params.gaussian_pair_count > 3u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_3);
    if(g_set0.params.gaussian_pair_count > 4u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_4);
    if(g_set0.params.gaussian_pair_count > 5u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_5);
    if(g_set0.params.gaussian_pair_count > 6u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_6);
    if(g_set0.params.gaussian_pair_count > 7u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_7);
    if(g_set0.params.gaussian_pair_count > 8u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_8);
    if(g_set0.params.gaussian_pair_count > 9u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_9);
    if(g_set0.params.gaussian_pair_count > 10u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_10);
    if(g_set0.params.gaussian_pair_count > 11u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_11);
    if(g_set0.params.gaussian_pair_count > 12u)
        color += gaussian_pair(uv, g_set0.params.gaussian_pair_12);
    return color;
}

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    if(dispatch_thread_id.x >= g_set0.params.dst_width ||
        dispatch_thread_id.y >= g_set0.params.dst_height)
    {
        return;
    }

    float2 destination_uv = (float2{float(dispatch_thread_id.x), float(dispatch_thread_id.y)} +
        float2{0.5f, 0.5f}) /
        float2{float(g_set0.params.dst_width), float(g_set0.params.dst_height)};
    float2 source_uv = g_set0.params.source_uv_origin +
        destination_uv * g_set0.params.source_uv_size;
    float4 color;
    if(g_set0.params.filter_mode == 1u)
    {
        color = downsample_tent(source_uv);
    }
    else if(g_set0.params.filter_mode == 2u)
    {
        color = gaussian_filter(source_uv);
    }
    else
    {
        color = sample_source(source_uv);
    }
    g_set0.destination.Store(dispatch_thread_id.xy, color);
}
