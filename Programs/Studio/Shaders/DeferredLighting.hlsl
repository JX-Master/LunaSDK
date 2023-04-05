#include "BRDF.hlsl"
#include "CameraParams.hlsl"

cbuffer LightingModeParams : register(b1)
{
    uint lighting_mode;
};

static const uint LIGHTING_MODE_LIT = 0;
static const uint LIGHTING_MODE_EMISSIVE = 1;
static const uint LIGHTING_MODE_DIFFUSE_LIGHTING = 2;
static const uint LIGHTING_MODE_SPECULAR_LIGHTING = 3;
static const uint LIGHTING_MODE_AMBIENT_DIFFUSE_LIGHTING = 4;
static const uint LIGHTING_MODE_AMBIENT_SPECULAR_LIGHTING = 5;

struct LightParams
{
    float3 strength;
    float attenuation_power;    // Only for point / spot light.

    float3 direction;   // Only for directional / spot light.
    uint type;          // 0 = directional, 1 = point, 2 = spot.

    float3 position;    // Only for point / spot light. In world space.
    float spot_attenuation_power;   // Only for spot light.
};

static const uint DIRECTIONAL_LIGHT = 0;
static const uint POINT_LIGHT = 1;
static const uint SPOT_LIGHT = 2;

StructuredBuffer<LightParams> g_light_params : register(t2);
Texture2D<float4> g_base_color_roughness : register(t3);
Texture2D<float4> g_normal_metallic : register(t4);
Texture2D<float4> g_emissive : register(t5);
Texture2D<float> g_depth : register(t6);
Texture2D<float4> g_skybox : register(t7);
Texture2D<float4> g_integrate_brdf : register(t8);
RWTexture2D<float4> g_light_buffer : register(u9);
SamplerState g_sampler : register(s10);

float3 fresnel_lerp(float3 specular_color_0, float3 specular_color_1, float l_dot_h)
{
    float t = pow(1.0f - l_dot_h, 5);
    return lerp(specular_color_0, specular_color_1, t);
}

[numthreads(8, 8, 1)]
void main(int3 dispatch_thread_id : SV_DispatchThreadID)
{
    float4 base_color_roughness = g_base_color_roughness[dispatch_thread_id.xy];
    float3 base_color = base_color_roughness.xyz;
    float roughness = base_color_roughness.w;
    float4 normal_metallic = g_normal_metallic[dispatch_thread_id.xy];
    float3 normal = normalize(normal_metallic.xyz * 2.0f - 1.0f);
    float metallic = normal_metallic.w;
    float3 emissive = g_emissive[dispatch_thread_id.xy].xyz;

    float depth = g_depth[dispatch_thread_id.xy];
    if (depth == 1.0f) return;

    float4 proj_space_position = float4(
        (float)dispatch_thread_id.x / (float)screen_width * 2.0 - 1.0,
        -(float)dispatch_thread_id.y / (float)screen_height * 2.0 + 1.0,
        depth, 1.0);

    proj_space_position = mul(proj_to_world, proj_space_position);
    float3 world_position = proj_space_position.xyz / proj_space_position.w;

    float4 camera_pos_world = mul(view_to_world, float4(0.0f, 0.0f, 0.0f, 1.0f));
    float3 view_dir = normalize(camera_pos_world.xyz - world_position);   // From frag to camera.

    uint num_lights;
    uint light_stride;
    g_light_params.GetDimensions(num_lights, light_stride);

    float3 specular_color = lerp(float3(0.04f, 0.08f, 0.08f), base_color.xyz, metallic);
    float3 diffuse_color = base_color.xyz * (1.0f - metallic);

    float nv = dot(normal, view_dir);

    float3 final_color = 0.0f;

    if (lighting_mode == LIGHTING_MODE_LIT ||
        lighting_mode == LIGHTING_MODE_DIFFUSE_LIGHTING ||
        lighting_mode == LIGHTING_MODE_SPECULAR_LIGHTING)
    {
        // Calculates lights contribution.
        for (uint index = 0; index < num_lights; ++index)
        {
            float3 light_color = g_light_params[index].strength;
            float3 light_dir;                    // frag to light.
            if (g_light_params[index].type == DIRECTIONAL_LIGHT) // directional.
            {
                light_dir = -normalize(g_light_params[index].direction);
            }
            else
            {
                light_dir = -normalize(world_position - g_light_params[index].position);
            }
            // Apply distance attenuation for point and spot light.
            if (g_light_params[index].type != DIRECTIONAL_LIGHT)
            {
                float dist = distance(world_position, g_light_params[index].position);
                float attenuation_power = max(g_light_params[index].attenuation_power, 0.000001f);
                float attentation = (dist / attenuation_power) * (dist / attenuation_power) + 1.0f;
                light_color /= attentation;
            }
            // Apply spot attenuation for spot light.
            if (g_light_params[index].type == SPOT_LIGHT)
            {
                float spot_attentation = pow(max(dot(-light_dir, g_light_params[index].direction), 0.0f), g_light_params[index].spot_attenuation_power);
                light_color *= spot_attentation;
            }

            float3 half_dir = normalize(light_dir + view_dir);
            float nl = dot(normal, light_dir);
            float nh = dot(normal, half_dir);
            float vh = dot(view_dir, half_dir);

            // Calculate diffuse term.
            float3 diffuse = light_diffuse_term(diffuse_color, specular_color);

            // Calculate specular term.
            float3 specular = light_specular_term(specular_color, nl, nv, nh, vh, roughness);
            float3 light_contrib;
            if (lighting_mode == LIGHTING_MODE_LIT)
            {
                light_contrib = diffuse + specular;
            }
            else if (lighting_mode == LIGHTING_MODE_DIFFUSE_LIGHTING)
            {
                light_contrib = diffuse;
            }
            else if (lighting_mode == LIGHTING_MODE_SPECULAR_LIGHTING)
            {
                light_contrib = specular;
            }
            final_color += light_contrib * light_color * max(nl, 0);
        }
    }

    // IBL
    const float SKY_BOX_MIPS = 5;
    float2 ibl_irradiance_uv = get_latlong_from_dir(normal);
    float3 ibl_irradiance = g_skybox.SampleLevel(g_sampler, ibl_irradiance_uv, (SKY_BOX_MIPS - 1)).xyz;
    float3 ibl_diffuse = ibl_irradiance * diffuse_color;

    float3 refl_dir = reflect(-view_dir, normal);
    float2 env_uv = get_latlong_from_dir(refl_dir);
    float3 env_color = g_skybox.SampleLevel(g_sampler, env_uv, roughness * (SKY_BOX_MIPS - 1)).xyz;
    float2 integrate_brdf = g_integrate_brdf.SampleLevel(g_sampler, float2(nv, roughness), 0).xy;
    float3 ibl_specular = env_color * (specular_color * integrate_brdf.x + integrate_brdf.y);

    //float one_minus_reflectivity = 1.0f - max(max(specular_color.r, specular_color.g), specular_color.b);
    //float grazing_term = saturate((1.0f - roughness) + (1.0f - one_minus_reflectivity));
    //float surface_reduction = 1.0f / (roughness * roughness + 1.0f);
    //float3 ibl_specular = surface_reduction * env_color.rgb * fresnel_lerp(specular_color, grazing_term, nv);

    // Composition.
    if (lighting_mode == LIGHTING_MODE_LIT)
    {
        final_color += emissive + ibl_specular + ibl_diffuse;
    }
    else if (lighting_mode == LIGHTING_MODE_EMISSIVE)
    {
        final_color = emissive;
    }
    else if (lighting_mode == LIGHTING_MODE_AMBIENT_DIFFUSE_LIGHTING)
    {
        final_color = ibl_diffuse;
    }
    else if(lighting_mode == LIGHTING_MODE_AMBIENT_SPECULAR_LIGHTING)
    {
        final_color = ibl_specular;
    }
    g_light_buffer[dispatch_thread_id.xy] = float4(final_color, 1.0f);
}