#pragma once
#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

float3 xyz(float4 value)
{
    return value.xyz;
}

float2 xy(uint3 value)
{
    return float2{float(value.x), float(value.y)};
}

uint2 xy_u(uint3 value)
{
    return value.xy;
}

float4 make_float4(float3 value, float w)
{
    return float4{value.x, value.y, value.z, w};
}

float luminance(float3 value)
{
    return dot(value, float3{0.2126f, 0.7152f, 0.0722f});
}

float2 get_latlong_from_dir(float3 dir)
{
    const float PI = 3.1415926f;
    dir = normalize(dir);
    float h = dir.y;
    float texcoord_v = 0.5f - asin(h) / PI;
    float texcoord_h = 0.5f - atan2(dir.z, dir.x) / (PI * 2.0f);
    return float2{texcoord_h, texcoord_v};
}

float3 get_dir_from_latlong(float2 uv)
{
    const float PI = 3.1415926f;
    float theta = PI * (0.5f - uv.y);
    float y = sin(theta);
    float cos_theta = cos(theta);
    float phi = uv.x * (PI * 2.0f);
    return normalize(float3{-cos(phi) * cos_theta, y, sin(phi) * cos_theta});
}
