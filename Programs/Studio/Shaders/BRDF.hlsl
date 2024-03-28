#include "Common.hlsl"

// Schlick with Spherical Gaussian approximation
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
float3 fresnel_schlick(float h_dot_v, float3 f0)
{
    float sphg = pow(2.0, (-5.55473 * h_dot_v - 6.98316) * h_dot_v);
    return f0 + (float3(1.0, 1.0, 1.0) - f0) * sphg;
}

// use GGX / Trowbridge-Reitz, same as Disney and Unreal 4
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
float ggx_normal_distrb(float h_dot_n, float roughness)
{
    float alpha = roughness * roughness;
    float tmp = alpha / max(1e-8, (h_dot_n * h_dot_n * (alpha * alpha - 1.0) + 1.0));
    return tmp * tmp / PI;
}

// Schlick with Smith-like choice of k
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
// visibility is a Cook-Torrance geometry function divided by (n.l)*(n.v)
float schlick_smith_visibility(float n_dot_l, float n_dot_v, float roughness)
{
    float k = max(0.001, roughness * roughness * 0.5);
    float lambda_v = ( n_dot_v * (1.0 - k) + k );
    float lambda_l = ( n_dot_l * (1.0 - k) + k );
    return 1.0 / (lambda_v * lambda_l);
}

float3 light_diffuse_term(float3 diffuse_color, float3 specular_color)
{
    return diffuse_color * (float3(1.0, 1.0, 1.0) - specular_color);
}

float3 light_specular_term(float3 specular_color, float n_dot_l, float n_dot_v, float h_dot_n, float h_dot_v, float roughness)
{
    float3 color = fresnel_schlick(h_dot_v, specular_color) * 
        ( ggx_normal_distrb(h_dot_n, roughness) * schlick_smith_visibility(n_dot_l, n_dot_v, roughness) / 4.0 );
    return color * PI;
}