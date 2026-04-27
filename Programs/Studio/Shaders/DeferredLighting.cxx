#include "BRDF.hxx"
#include <cppsl/texture.hxx>

struct CameraParams
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 proj_to_world;
    float4x4 view_to_world;
    uint screen_width;
    uint screen_height;
};

struct LightingParams
{
    uint lighting_mode;
    uint num_lights;
};

struct LightParams
{
    float3 strength;
    float attenuation_power;
    float3 direction;
    uint type;
    float3 position;
    float spot_attenuation_power;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CameraParams camera_params;

    [[cppsl::cbuffer, cppsl::binding(1)]]
    LightingParams lighting_params;

    [[cppsl::structured_buffer, cppsl::binding(2)]]
    const LightParams* g_light_params;

    [[cppsl::binding(3)]]
    Texture2D<float4> g_base_color_roughness;

    [[cppsl::binding(4)]]
    Texture2D<float4> g_normal_metallic;

    [[cppsl::binding(5)]]
    Texture2D<float4> g_emissive;

    [[cppsl::binding(6)]]
    DepthTexture2D<float> g_depth;

    [[cppsl::binding(7)]]
    Texture2D<float4> g_skybox;

    [[cppsl::binding(8)]]
    Texture2D<float4> g_integrate_brdf;

    [[cppsl::binding(9)]]
    RWTexture2D<float4> g_light_buffer;

    [[cppsl::binding(10)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = dispatch_thread_id.xy;
    float4 base_color_roughness = g_set0.g_base_color_roughness.Load(pixel);
    float3 base_color = base_color_roughness.xyz;
    float roughness = max(base_color_roughness.w, 0.001f);
    float4 normal_metallic = g_set0.g_normal_metallic.Load(pixel);
    float3 normal = normalize(normal_metallic.xyz * 2.0f - 1.0f);
    float metallic = normal_metallic.w;
    float3 emissive = g_set0.g_emissive.Load(pixel).xyz;

    float depth = g_set0.g_depth.Load(pixel);
    if (depth >= 0.999f)
    {
        return;
    }

    float4 proj_space_position = float4{
        float(dispatch_thread_id.x) / float(g_set0.camera_params.screen_width) * 2.0f - 1.0f,
        -(float(dispatch_thread_id.y) / float(g_set0.camera_params.screen_height) * 2.0f) + 1.0f,
        depth,
        1.0f};

    proj_space_position = mul(g_set0.camera_params.proj_to_world, proj_space_position);
    float3 world_position = proj_space_position.xyz / proj_space_position.w;

    float4 camera_pos_world = mul(g_set0.camera_params.view_to_world, float4{0.0f, 0.0f, 0.0f, 1.0f});
    float3 view_dir = normalize(camera_pos_world.xyz - world_position);

    float3 specular_color = lerp(float3{0.04f, 0.08f, 0.08f}, base_color, metallic);
    float3 diffuse_color = base_color * (1.0f - metallic);
    float nv = max(dot(normal, view_dir), 0.000001f);
    float3 final_color = float3{0.0f, 0.0f, 0.0f};

    if (g_set0.lighting_params.lighting_mode == 0u ||
        g_set0.lighting_params.lighting_mode == 2u ||
        g_set0.lighting_params.lighting_mode == 3u)
    {
        for (uint index = 0u; index < g_set0.lighting_params.num_lights; ++index)
        {
            float3 light_color = g_set0.g_light_params[index].strength;
            float3 light_dir;
            if (g_set0.g_light_params[index].type == 0u)
            {
                light_dir = -normalize(g_set0.g_light_params[index].direction);
            }
            else
            {
                light_dir = -normalize(world_position - g_set0.g_light_params[index].position);
            }

            if (g_set0.g_light_params[index].type != 0u)
            {
                float dist = distance(world_position, g_set0.g_light_params[index].position);
                float attenuation_power = max(g_set0.g_light_params[index].attenuation_power, 0.000001f);
                float attenuation = (dist / attenuation_power) * (dist / attenuation_power) + 1.0f;
                light_color /= attenuation;
            }

            if (g_set0.g_light_params[index].type == 2u)
            {
                float spot_attenuation = pow(max(dot(-light_dir, g_set0.g_light_params[index].direction), 0.0f), g_set0.g_light_params[index].spot_attenuation_power);
                light_color *= spot_attenuation;
            }

            float3 half_dir = normalize(light_dir + view_dir);
            float nl = dot(normal, light_dir);
            if (nl <= 0.0f)
            {
                continue;
            }
            float nh = max(dot(normal, half_dir), 0.0f);
            float vh = max(dot(view_dir, half_dir), 0.0f);

            float3 diffuse = light_diffuse_term(diffuse_color, specular_color);
            float3 specular = light_specular_term(specular_color, nl, nv, nh, vh, roughness);
            float3 light_contrib = float3{0.0f, 0.0f, 0.0f};
            if (g_set0.lighting_params.lighting_mode == 0u)
            {
                light_contrib = diffuse + specular;
            }
            else if (g_set0.lighting_params.lighting_mode == 2u)
            {
                light_contrib = diffuse;
            }
            else if (g_set0.lighting_params.lighting_mode == 3u)
            {
                light_contrib = specular;
            }
            final_color += light_contrib * light_color * nl;
        }
    }

    const float SKY_BOX_MIPS = 5.0f;
    float2 ibl_irradiance_uv = get_latlong_from_dir(normal);
    float3 ibl_irradiance = g_set0.g_skybox.SampleLevel(g_set0.g_sampler, ibl_irradiance_uv, SKY_BOX_MIPS - 1.0f).xyz;
    float3 ibl_diffuse = ibl_irradiance * diffuse_color;

    float3 refl_dir = reflect(-view_dir, normal);
    float2 env_uv = get_latlong_from_dir(refl_dir);
    float3 env_color = g_set0.g_skybox.SampleLevel(g_set0.g_sampler, env_uv, roughness * (SKY_BOX_MIPS - 1.0f)).xyz;
    float4 integrate_brdf_sample = g_set0.g_integrate_brdf.SampleLevel(g_set0.g_sampler, float2{nv, roughness}, 0.0f);
    float3 ibl_specular = env_color * (specular_color * integrate_brdf_sample.x + integrate_brdf_sample.y);

    if (g_set0.lighting_params.lighting_mode == 0u)
    {
        final_color += emissive + ibl_specular + ibl_diffuse;
    }
    else if (g_set0.lighting_params.lighting_mode == 1u)
    {
        final_color = emissive;
    }
    else if (g_set0.lighting_params.lighting_mode == 4u)
    {
        final_color = ibl_diffuse;
    }
    else if (g_set0.lighting_params.lighting_mode == 5u)
    {
        final_color = ibl_specular;
    }

    g_set0.g_light_buffer.Store(pixel, make_float4(final_color, 1.0f));
}
