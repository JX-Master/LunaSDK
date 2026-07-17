#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct ShadowParams
{
    float4 draw_rect;
    float4 source_rect;
    float4 clip_rect;
    float4 color;
    float4 shadow_params;
    float4 screen_params;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    ShadowParams params;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 screen_position;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 color;
};

float rounded_rect_distance(float2 point, float4 rect, float radius)
{
    float2 half_size = rect.zw * 0.5f;
    float2 center = rect.xy + half_size;
    float clamped_radius = min(radius, min(half_size.x, half_size.y));
    float2 distance_to_corner = abs(point - center) - (half_size - clamped_radius);
    float2 outside = max(distance_to_corner, float2{0.0f, 0.0f});
    return sqrt(dot(outside, outside)) +
        min(max(distance_to_corner.x, distance_to_corner.y), 0.0f) - clamped_radius;
}

float normal_distribution_cdf(float value)
{
    float absolute_value = abs(value);
    float t = 1.0f / (1.0f + 0.2316419f * absolute_value);
    float density = 0.3989422804f * exp2(-0.72134752f * absolute_value * absolute_value);
    float tail = density * t * (0.319381530f + t * (-0.356563782f + t *
        (1.781477937f + t * (-1.821255978f + t * 1.330274429f))));
    return value >= 0.0f ? 1.0f - tail : tail;
}

float blurred_mask_coverage(float distance, float softness)
{
    if(softness <= 0.0001f)
    {
        return saturate(0.5f - distance);
    }
    return normal_distribution_cdf(-distance / softness);
}

[[cppsl::fragment]]
PSOutput ps_main(PSInput pixel)
{
    float radius = g_set0.params.shadow_params.x;
    float softness = g_set0.params.shadow_params.y;
    bool inner = g_set0.params.shadow_params.z > 0.5f;
    bool clipped = g_set0.params.shadow_params.w > 0.5f;
    float distance = rounded_rect_distance(pixel.screen_position, g_set0.params.source_rect, radius);
    float coverage;
    if(inner)
    {
        float source_coverage = saturate(0.5f - rounded_rect_distance(
            pixel.screen_position, g_set0.params.draw_rect, g_set0.params.screen_params.z));
        coverage = source_coverage * blurred_mask_coverage(-distance, softness);
    }
    else
    {
        coverage = blurred_mask_coverage(distance, softness);
    }
    if(clipped)
    {
        float2 clip_min = g_set0.params.clip_rect.xy;
        float2 clip_max = clip_min + g_set0.params.clip_rect.zw;
        bool inside_clip = pixel.screen_position.x >= clip_min.x && pixel.screen_position.y >= clip_min.y &&
            pixel.screen_position.x < clip_max.x && pixel.screen_position.y < clip_max.y;
        coverage *= inside_clip ? 1.0f : 0.0f;
    }
    PSOutput result;
    result.color = g_set0.params.color;
    result.color.w *= coverage;
    return result;
}
