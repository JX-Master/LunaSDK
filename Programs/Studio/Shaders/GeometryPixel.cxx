#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

void discard_fragment();

struct CBParam
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 proj_to_world;
    float4x4 view_to_world;
    float4 env_light_color;
    uint screen_width;
    uint screen_height;
};

struct MaterialParameters
{
    float emissive_intensity;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CBParam cb_param;

    [[cppsl::binding(2)]]
    Texture2D<float4> g_base_color;

    [[cppsl::binding(3)]]
    Texture2D<float4> g_roughness;

    [[cppsl::binding(4)]]
    Texture2D<float4> g_normal;

    [[cppsl::binding(5)]]
    Texture2D<float4> g_metallic;

    [[cppsl::binding(6)]]
    Texture2D<float4> g_emissive;

    [[cppsl::binding(7)]]
    SamplerState g_sampler;

    [[cppsl::structured_buffer, cppsl::binding(8)]]
    const MaterialParameters* g_material_params;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float3 normal;
    [[cppsl::location(2)]] float3 tangent;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] float4 color;
    [[cppsl::location(5)]] float3 world_position;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 base_color_roughness;
    [[cppsl::location(1)]] float4 normal_metallic;
    [[cppsl::location(2)]] float4 emissive;
};

float3 normal_tangent_to_world(float3 normal_map, float3 normal_world, float3 tangent_world)
{
    float3 n = normal_world;
    float3 t = normalize(tangent_world - dot(tangent_world, n) * n);
    float3 b = cross(n, t);
    return normalize(t * normal_map.x + b * normal_map.y + n * normal_map.z);
}

[[cppsl::pixel]]
PSOutput ps_main(PSInput i)
{
    float2 texcoord = float2{i.texcoord.x, 1.0f - i.texcoord.y};
    float4 base_color = g_set0.g_base_color.Sample(g_set0.g_sampler, texcoord);
    if (base_color.w < 0.1f)
    {
        discard_fragment();
    }

    float roughness = g_set0.g_roughness.Sample(g_set0.g_sampler, texcoord).x;
    float3 normal = normalize(xyz(g_set0.g_normal.Sample(g_set0.g_sampler, texcoord)) - 0.5f);
    float metallic = g_set0.g_metallic.Sample(g_set0.g_sampler, texcoord).x;
    float4 emissive = g_set0.g_emissive.Sample(g_set0.g_sampler, texcoord);

    float3 base_normal = normalize(i.normal);
    float3 base_tangent = normalize(i.tangent);
    normal = normal_tangent_to_world(normal, base_normal, base_tangent);

    PSOutput o;
    o.base_color_roughness = float4{base_color.x, base_color.y, base_color.z, roughness};
    float3 encoded_normal = max(normal * 0.5f + 0.5f, 0.0f);
    o.normal_metallic = float4{encoded_normal.x, encoded_normal.y, encoded_normal.z, metallic};
    o.emissive = float4{
        emissive.x * g_set0.g_material_params[0].emissive_intensity,
        emissive.y * g_set0.g_material_params[0].emissive_intensity,
        emissive.z * g_set0.g_material_params[0].emissive_intensity,
        0.0f};
    return o;
}
