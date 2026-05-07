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
    float4 mul(float4 v, float4x4 m);

    float sin(float value);
    float cos(float value);
    float tan(float value);
    float asin(float value);
    float atan2(float y, float x);
    float sqrt(float value);
    float pow(float value, float exponent);
    float log2(float value);
    float exp2(float value);
    float abs(float value);
    float2 abs(float2 value);
    float3 abs(float3 value);
    float4 abs(float4 value);
    float min(float a, float b);
    float2 min(float2 a, float2 b);
    float3 min(float3 a, float3 b);
    float4 min(float4 a, float4 b);
    float2 min(float2 a, float b);
    float3 min(float3 a, float b);
    float4 min(float4 a, float b);
    float max(float a, float b);
    float2 max(float2 a, float2 b);
    float3 max(float3 a, float3 b);
    float4 max(float4 a, float4 b);
    float2 max(float2 a, float b);
    float3 max(float3 a, float b);
    float4 max(float4 a, float b);
    uint max(uint a, uint b);
    int max(int a, int b);
    float saturate(float value);
    float2 saturate(float2 value);
    float3 saturate(float3 value);
    float4 saturate(float4 value);
    float2 fwidth(float2 value);
    float4 fwidth(float4 value);
    bool any(bool2 value);
    bool any(bool3 value);
    bool any(bool4 value);
    float distance(float3 a, float3 b);
    float3 reflect(float3 incident, float3 normal);
    float lerp(float a, float b, float t);
    float2 lerp(float2 a, float2 b, float t);
    float3 lerp(float3 a, float3 b, float t);
    float4 lerp(float4 a, float4 b, float t);

    float2 operator-(float2 value);
    float3 operator-(float3 value);
    float4 operator-(float4 value);

    float2 operator+(float2 a, float2 b);
    float3 operator+(float3 a, float3 b);
    float4 operator+(float4 a, float4 b);

    float2 operator+(float2 a, float b);
    float3 operator+(float3 a, float b);
    float4 operator+(float4 a, float b);
    float2 operator+(float a, float2 b);
    float3 operator+(float a, float3 b);
    float4 operator+(float a, float4 b);

    float2 operator-(float2 a, float2 b);
    float3 operator-(float3 a, float3 b);
    float4 operator-(float4 a, float4 b);

    float2 operator-(float2 a, float b);
    float3 operator-(float3 a, float b);
    float4 operator-(float4 a, float b);
    float2 operator-(float a, float2 b);
    float3 operator-(float a, float3 b);
    float4 operator-(float a, float4 b);

    float2 operator*(float2 a, float2 b);
    float3 operator*(float3 a, float3 b);
    float4 operator*(float4 a, float4 b);

    float2 operator*(float2 a, float b);
    float3 operator*(float3 a, float b);
    float4 operator*(float4 a, float b);

    float2 operator*(float a, float2 b);
    float3 operator*(float a, float3 b);
    float4 operator*(float a, float4 b);

    float2 operator/(float2 a, float2 b);
    float3 operator/(float3 a, float3 b);
    float4 operator/(float4 a, float4 b);
    float2 operator/(float2 a, float b);
    float3 operator/(float3 a, float b);
    float4 operator/(float4 a, float b);
    float2 operator/(float a, float2 b);
    float3 operator/(float a, float3 b);
    float4 operator/(float a, float4 b);

    float2& operator+=(float2& a, float2 b);
    float3& operator+=(float3& a, float3 b);
    float4& operator+=(float4& a, float4 b);
    float2& operator+=(float2& a, float b);
    float3& operator+=(float3& a, float b);
    float4& operator+=(float4& a, float b);
    float2& operator-=(float2& a, float2 b);
    float3& operator-=(float3& a, float3 b);
    float4& operator-=(float4& a, float4 b);
    float2& operator-=(float2& a, float b);
    float3& operator-=(float3& a, float b);
    float4& operator-=(float4& a, float b);
    float2& operator*=(float2& a, float2 b);
    float3& operator*=(float3& a, float3 b);
    float4& operator*=(float4& a, float4 b);
    float2& operator*=(float2& a, float b);
    float3& operator*=(float3& a, float b);
    float4& operator*=(float4& a, float b);
    float2& operator/=(float2& a, float b);
    float3& operator/=(float3& a, float b);
    float4& operator/=(float4& a, float b);

    bool2 operator!=(float2 a, float2 b);
    bool3 operator!=(float3 a, float3 b);
    bool4 operator!=(float4 a, float4 b);

    float clamp(float value, float min_value, float max_value);
    float2 clamp(float2 value, float min_value, float max_value);
    float3 clamp(float3 value, float min_value, float max_value);
    float4 clamp(float4 value, float min_value, float max_value);
    float2 clamp(float2 value, float2 min_value, float2 max_value);
    float3 clamp(float3 value, float3 min_value, float3 max_value);
    float4 clamp(float4 value, float4 min_value, float4 max_value);
    int2 clamp(int2 value, int2 min_value, int2 max_value);
}
