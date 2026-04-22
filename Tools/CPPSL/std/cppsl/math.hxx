#pragma once
#include <cppsl/core.hxx>

namespace cppsl
{
    float dot(float2 a, float2 b);
    float dot(float3 a, float3 b);
    float dot(float4 a, float4 b);

    float3 cross(float3 a, float3 b);
    float3 normalize(float3 v);
    float4 mul(float4x4 m, float4 v);
}
