#pragma once
#include "StudioCommon.hxx"

float radical_inverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10f;
}

float2 hammersley2d(uint i, uint n)
{
    return float2{float(i) / float(n), radical_inverse_VdC(i)};
}

float3 importance_sample_ggx(float2 xi, float3 n, float roughness)
{
    const float PI = 3.1415926f;
    float a = roughness * roughness;
    float phi = 2.0f * PI * xi.x;
    float cos_theta = sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));
    float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
    float3 h = float3{cos(phi) * sin_theta, cos_theta, -sin(phi) * sin_theta};

    float3 up = abs(n.y) < 0.999f ? float3{0.0f, 1.0f, 0.0f} : float3{1.0f, 0.0f, 0.0f};
    float3 tangent = normalize(cross(n, up));
    float3 bitangent = cross(n, tangent);
    float3 sample_vec = tangent * h.x + n * h.y + bitangent * h.z;
    return normalize(sample_vec);
}

float schlick_ggx_geometry(float n_dot_v, float roughness)
{
    float k = roughness * roughness / 2.0f;
    return n_dot_v / (n_dot_v * (1.0f - k) + k);
}

float geometry_smith(float n_dot_v, float n_dot_l, float roughness)
{
    float g1 = schlick_ggx_geometry(n_dot_v, roughness);
    float g2 = schlick_ggx_geometry(n_dot_l, roughness);
    return g1 * g2;
}

float ggx_normal_distrb(float h_dot_n, float roughness)
{
    const float PI = 3.1415926f;
    float alpha = roughness * roughness;
    float tmp = alpha / max(1e-8f, (h_dot_n * h_dot_n * (alpha * alpha - 1.0f) + 1.0f));
    return tmp * tmp / PI;
}

float3 get_integrate_brdf(float n_dot_v, float roughness)
{
    float3 v = float3{0.0f, sqrt(1.0f - n_dot_v * n_dot_v), n_dot_v};
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float3 n = float3{0.0f, 0.0f, 1.0f};

    const uint sample_count = 1024u;
    for (uint i = 0u; i < sample_count; ++i)
    {
        float2 xi = hammersley2d(i, sample_count);
        float3 h = importance_sample_ggx(xi, n, roughness);
        float3 l = normalize(2.0f * dot(v, h) * h - v);

        float n_dot_l = max(l.z, 0.0f);
        float n_dot_v_local = max(v.z, 0.0f);
        float n_dot_h = max(h.z, 0.0f);
        float v_dot_h = max(dot(v, h), 0.0f);

        if (n_dot_l > 0.0f)
        {
            float g = geometry_smith(n_dot_v_local, n_dot_l, roughness);
            float g_vis = (g * v_dot_h) / (n_dot_h * n_dot_v_local);
            float fc = pow(1.0f - v_dot_h, 5.0f);
            a += (1.0f - fc) * g_vis;
            b += fc * g_vis;
        }
    }

    return float3{a, b, c} / float(sample_count);
}
