#include "GeometryCommon.hlsl"
struct PSInput
{
    [[vk::location(0)]]
    float4 position : SV_POSITION;
    [[vk::location(1)]]
    float3 normal   : NORMAL;
    [[vk::location(2)]]
    float3 tangent  : TANGENT;
    [[vk::location(3)]]
    float2 texcoord : TEXCOORD;
    [[vk::location(4)]]
    float4 color    : COLOR;
    [[vk::location(5)]]
    float3 world_position : POSITION;
};

struct PSOutput
{
    // RGB: base color, A: roughness
    [[vk::location(0)]]
    float4 base_color_roughness : SV_Target0;
    // RGB: normal, A: metallic
    [[vk::location(1)]]
    float4 normal_metallic : SV_Target1;
    // RGB: emissive, A: unused.
    [[vk::location(2)]]
    float4 emissive : SV_Target2;
};

float3 normal_tangent_to_world(float3 normal_map, float3 normal_world, float3 tangent_world)
{
    float3 n = normal_world;
    float3 t = normalize(tangent_world - dot(tangent_world, n) * n);
    float3 b = cross(n, t);
    float3x3 tbn = float3x3(t, b, n);
    return mul(normal_map, tbn);
}

PSOutput main(PSInput i)
{
    float2 texcoord = float2(i.texcoord.x, 1.0f - i.texcoord.y);

    // Sample every texture.
    float4 base_color = g_base_color.Sample(g_sampler, texcoord);
    clip(base_color.w - 0.1f);
    float roughness = g_roughness.Sample(g_sampler, texcoord).x;
    float3 normal = normalize(g_normal.Sample(g_sampler, texcoord).xyz - 0.5f);
    float metallic = g_metallic.Sample(g_sampler, texcoord).x;
    float4 emissive = g_emissive.Sample(g_sampler, texcoord);

    // Calculates per-model informations.
    float3 base_normal = normalize(i.normal);
    float3 base_tangent = normalize(i.tangent);
    float4 camera_pos_world = mul(view_to_world, float4(0.0f, 0.0f, 0.0f, 1.0f));
    float3 view_dir = normalize(camera_pos_world.xyz - i.world_position);   // From frag to camera.

    // Apply normal map to model normal.
    normal = normal_tangent_to_world(normal, base_normal, base_tangent);

    PSOutput o;
    o.base_color_roughness.xyz = base_color.xyz;
    o.base_color_roughness.w = roughness;
    o.normal_metallic.xyz = max(normal * 0.5f + 0.5f, 0.0f);
    o.normal_metallic.w = metallic;
    o.emissive.xyz = emissive.xyz * g_material_params[0].emissive_intensity;
    o.emissive.w = 0.0f;
    return o;
}