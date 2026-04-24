#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct SkyboxParams
{
    float4x4 g_view_to_world;
    float g_fov;
    uint g_width;
    uint g_height;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
SkyboxParams skybox_params;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float4> g_skybox;

[[cppsl::desc_set(0), cppsl::binding(2)]]
Texture2D<float> g_depth;

[[cppsl::desc_set(0), cppsl::binding(3)]]
RWTexture2D<float4> g_lighting_tex;

[[cppsl::desc_set(0), cppsl::binding(4)]]
SamplerState g_sampler;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    if (g_depth.Load(pixel) != 1.0f)
    {
        return;
    }

    float focus_len = float(skybox_params.g_width) / (2.0f * tan(skybox_params.g_fov / 2.0f));
    float3 world_dir = normalize(float3{
        float(dispatch_thread_id.x) - float(skybox_params.g_width / 2u),
        -(float(dispatch_thread_id.y) - float(skybox_params.g_height / 2u)),
        focus_len});
    world_dir = xyz(mul(skybox_params.g_view_to_world, float4{world_dir.x, world_dir.y, world_dir.z, 0.0f}));

    float2 env_uv = get_latlong_from_dir(world_dir);
    float4 src_color = g_skybox.SampleLevel(g_sampler, env_uv, 0.0f);
    g_lighting_tex.Store(pixel, float4{src_color.x, src_color.y, src_color.z, 1.0f});
}
