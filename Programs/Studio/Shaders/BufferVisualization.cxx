#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct VisualizationParams
{
    uint vis_type;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    VisualizationParams visualization_params;

    [[cppsl::binding(1)]]
    Texture2D<float4> g_base_color_roughness;

    [[cppsl::binding(2)]]
    Texture2D<float4> g_normal_metallic;

    [[cppsl::binding(3)]]
    DepthTexture2D<float> g_depth;

    [[cppsl::binding(4)]]
    RWTexture2D<float4> g_dst;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float4 base_color_roughness = g_set0.g_base_color_roughness.Load(pixel);
    float3 base_color = xyz(base_color_roughness);
    float roughness = base_color_roughness.w;
    float4 normal_metallic = g_set0.g_normal_metallic.Load(pixel);
    float metallic = normal_metallic.w;
    float depth = g_set0.g_depth.Load(pixel);

    if (g_set0.visualization_params.vis_type == 0u)
    {
        g_set0.g_dst.Store(pixel, make_float4(base_color, 1.0f));
    }
    else if (g_set0.visualization_params.vis_type == 1u)
    {
        g_set0.g_dst.Store(pixel, float4{normal_metallic.x, normal_metallic.y, normal_metallic.z, 1.0f});
    }
    else if (g_set0.visualization_params.vis_type == 2u)
    {
        g_set0.g_dst.Store(pixel, float4{roughness, roughness, roughness, 1.0f});
    }
    else if (g_set0.visualization_params.vis_type == 3u)
    {
        g_set0.g_dst.Store(pixel, float4{metallic, metallic, metallic, 1.0f});
    }
    else if (g_set0.visualization_params.vis_type == 4u)
    {
        g_set0.g_dst.Store(pixel, float4{depth, depth, depth, 1.0f});
    }
}
