#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

template <typename T>
T AddBias(T value, T bias)
{
    return value + bias;
}

template <typename T>
T SelectValue(bool use_left, T left, T right)
{
    return use_left ? left : right;
}

template <typename T>
T Combine(T value)
{
    return value;
}

template <typename T>
T Combine(T left, T right)
{
    return left + right;
}

struct VSInput
{
    [[cppsl::location(0)]]
    float3 position;

    [[cppsl::location(1)]]
    float weight;
};

struct VSOutput
{
    [[cppsl::position]]
    float4 position;

    [[cppsl::location(0)]]
    float4 color;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    VSOutput o;
    float scalar = AddBias(v.weight, 0.25f);
    float3 vector = AddBias(v.position, float3(0.5f, 0.5f, 0.5f));
    float selected = SelectValue(true, scalar, v.weight);
    float combined = Combine(v.weight) + Combine(v.weight, 0.5f);
    o.position = float4(vector, 1.0f);
    o.color = float4(vector.x, vector.y, vector.z, selected + combined);
    return o;
}
