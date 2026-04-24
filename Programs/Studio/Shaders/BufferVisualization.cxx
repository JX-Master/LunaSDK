#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct VisualizationParams
{
    uint vis_type;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
VisualizationParams visualization_params;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float4> g_base_color_roughness;

[[cppsl::desc_set(0), cppsl::binding(2)]]
Texture2D<float4> g_normal_metallic;

[[cppsl::desc_set(0), cppsl::binding(3)]]
Texture2D<float> g_depth;

[[cppsl::desc_set(0), cppsl::binding(4)]]
RWTexture2D<float4> g_dst;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float4 base_color_roughness = g_base_color_roughness.Load(pixel);
    float3 base_color = xyz(base_color_roughness);
    float roughness = base_color_roughness.w;
    float4 normal_metallic = g_normal_metallic.Load(pixel);
    float metallic = normal_metallic.w;
    float depth = g_depth.Load(pixel);

    if (visualization_params.vis_type == 0u)
    {
        g_dst.Store(pixel, make_float4(base_color, 1.0f));
    }
    else if (visualization_params.vis_type == 1u)
    {
        g_dst.Store(pixel, float4{normal_metallic.x, normal_metallic.y, normal_metallic.z, 1.0f});
    }
    else if (visualization_params.vis_type == 2u)
    {
        g_dst.Store(pixel, float4{roughness, roughness, roughness, 1.0f});
    }
    else if (visualization_params.vis_type == 3u)
    {
        g_dst.Store(pixel, float4{metallic, metallic, metallic, 1.0f});
    }
    else if (visualization_params.vis_type == 4u)
    {
        g_dst.Store(pixel, float4{depth, depth, depth, 1.0f});
    }
}
