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

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
CameraParams camera_params;

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(1)]]
LightingParams lighting_params;

[[cppsl::structured_buffer, cppsl::desc_set(0), cppsl::binding(2)]]
const LightParams* g_light_params;

[[cppsl::desc_set(0), cppsl::binding(3)]]
Texture2D<float4> g_base_color_roughness;

[[cppsl::desc_set(0), cppsl::binding(4)]]
Texture2D<float4> g_normal_metallic;

[[cppsl::desc_set(0), cppsl::binding(5)]]
Texture2D<float4> g_emissive;

[[cppsl::desc_set(0), cppsl::binding(6)]]
Texture2D<float> g_depth;

[[cppsl::desc_set(0), cppsl::binding(7)]]
Texture2D<float4> g_skybox;

[[cppsl::desc_set(0), cppsl::binding(8)]]
Texture2D<float4> g_integrate_brdf;

[[cppsl::desc_set(0), cppsl::binding(9)]]
RWTexture2D<float4> g_light_buffer;

[[cppsl::desc_set(0), cppsl::binding(10)]]
SamplerState g_sampler;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float4 base_color_roughness = g_base_color_roughness.Load(pixel);
    float3 base_color = xyz(base_color_roughness);
    float roughness = base_color_roughness.w;
    float4 normal_metallic = g_normal_metallic.Load(pixel);
    float3 normal = normalize(xyz(normal_metallic) * 2.0f - 1.0f);
    float metallic = normal_metallic.w;
    float3 emissive = xyz(g_emissive.Load(pixel));

    float depth = g_depth.Load(pixel);
    if (depth == 1.0f)
    {
        return;
    }

    float4 proj_space_position = float4{
        float(dispatch_thread_id.x) / float(camera_params.screen_width) * 2.0f - 1.0f,
        -(float(dispatch_thread_id.y) / float(camera_params.screen_height) * 2.0f) + 1.0f,
        depth,
        1.0f};

    proj_space_position = mul(camera_params.proj_to_world, proj_space_position);
    float3 world_position = xyz(proj_space_position) / proj_space_position.w;

    float4 camera_pos_world = mul(camera_params.view_to_world, float4{0.0f, 0.0f, 0.0f, 1.0f});
    float3 view_dir = normalize(xyz(camera_pos_world) - world_position);

    float3 specular_color = lerp(float3{0.04f, 0.08f, 0.08f}, base_color, metallic);
    float3 diffuse_color = base_color * (1.0f - metallic);
    float nv = dot(normal, view_dir);
    float3 final_color = float3{0.0f, 0.0f, 0.0f};

    if (lighting_params.lighting_mode == 0u ||
        lighting_params.lighting_mode == 2u ||
        lighting_params.lighting_mode == 3u)
    {
        for (uint index = 0u; index < lighting_params.num_lights; ++index)
        {
            float3 light_color = g_light_params[index].strength;
            float3 light_dir;
            if (g_light_params[index].type == 0u)
            {
                light_dir = -normalize(g_light_params[index].direction);
            }
            else
            {
                light_dir = -normalize(world_position - g_light_params[index].position);
            }

            if (g_light_params[index].type != 0u)
            {
                float dist = distance(world_position, g_light_params[index].position);
                float attenuation_power = max(g_light_params[index].attenuation_power, 0.000001f);
                float attenuation = (dist / attenuation_power) * (dist / attenuation_power) + 1.0f;
                light_color /= attenuation;
            }

            if (g_light_params[index].type == 2u)
            {
                float spot_attenuation = pow(max(dot(-light_dir, g_light_params[index].direction), 0.0f), g_light_params[index].spot_attenuation_power);
                light_color *= spot_attenuation;
            }

            float3 half_dir = normalize(light_dir + view_dir);
            float nl = dot(normal, light_dir);
            if (nl <= 0.0f)
            {
                continue;
            }
            float nh = dot(normal, half_dir);
            float vh = dot(view_dir, half_dir);

            float3 diffuse = light_diffuse_term(diffuse_color, specular_color);
            float3 specular = light_specular_term(specular_color, nl, nv, nh, vh, roughness);
            float3 light_contrib = float3{0.0f, 0.0f, 0.0f};
            if (lighting_params.lighting_mode == 0u)
            {
                light_contrib = diffuse + specular;
            }
            else if (lighting_params.lighting_mode == 2u)
            {
                light_contrib = diffuse;
            }
            else if (lighting_params.lighting_mode == 3u)
            {
                light_contrib = specular;
            }
            final_color += light_contrib * light_color * nl;
        }
    }

    const float SKY_BOX_MIPS = 5.0f;
    float2 ibl_irradiance_uv = get_latlong_from_dir(normal);
    float3 ibl_irradiance = xyz(g_skybox.SampleLevel(g_sampler, ibl_irradiance_uv, SKY_BOX_MIPS - 1.0f));
    float3 ibl_diffuse = ibl_irradiance * diffuse_color;

    float3 refl_dir = reflect(-view_dir, normal);
    float2 env_uv = get_latlong_from_dir(refl_dir);
    float3 env_color = xyz(g_skybox.SampleLevel(g_sampler, env_uv, roughness * (SKY_BOX_MIPS - 1.0f)));
    float4 integrate_brdf_sample = g_integrate_brdf.SampleLevel(g_sampler, float2{nv, roughness}, 0.0f);
    float3 ibl_specular = env_color * (specular_color * integrate_brdf_sample.x + integrate_brdf_sample.y);

    if (lighting_params.lighting_mode == 0u)
    {
        final_color += emissive + ibl_specular + ibl_diffuse;
    }
    else if (lighting_params.lighting_mode == 1u)
    {
        final_color = emissive;
    }
    else if (lighting_params.lighting_mode == 4u)
    {
        final_color = ibl_diffuse;
    }
    else if (lighting_params.lighting_mode == 5u)
    {
        final_color = ibl_specular;
    }

    g_light_buffer.Store(pixel, make_float4(final_color, 1.0f));
}
