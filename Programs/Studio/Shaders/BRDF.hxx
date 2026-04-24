#pragma once
#include "IBLCommon.hxx"

float3 fresnel_schlick(float h_dot_v, float3 f0)
{
    float sphg = pow(2.0f, (-5.55473f * h_dot_v - 6.98316f) * h_dot_v);
    return f0 + (float3{1.0f, 1.0f, 1.0f} - f0) * sphg;
}

float schlick_smith_visibility(float n_dot_l, float n_dot_v, float roughness)
{
    float k = max(0.001f, roughness * roughness * 0.5f);
    float lambda_v = (n_dot_v * (1.0f - k) + k);
    float lambda_l = (n_dot_l * (1.0f - k) + k);
    return 1.0f / (lambda_v * lambda_l);
}

float3 light_diffuse_term(float3 diffuse_color, float3 specular_color)
{
    return diffuse_color * (float3{1.0f, 1.0f, 1.0f} - specular_color);
}

float3 light_specular_term(float3 specular_color, float n_dot_l, float n_dot_v, float h_dot_n, float h_dot_v, float roughness)
{
    const float PI = 3.1415926f;
    float3 color = fresnel_schlick(h_dot_v, specular_color) *
        (ggx_normal_distrb(h_dot_n, roughness) * schlick_smith_visibility(n_dot_l, n_dot_v, roughness) / 4.0f);
    return color * PI;
}
