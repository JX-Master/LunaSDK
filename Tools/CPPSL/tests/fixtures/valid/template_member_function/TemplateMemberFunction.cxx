#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct Biaser
{
    float base;

    template <typename T>
    T Add(T value, T bias) const
    {
        return value + bias;
    }

    template <typename T>
    T Combine(T value) const
    {
        return value;
    }

    template <typename T>
    T Combine(T left, T right) const
    {
        return left + right;
    }
};

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
    Biaser biaser;
    biaser.base = 0.25f;

    VSOutput o;
    float scalar = biaser.Add(v.weight, biaser.base);
    float3 vector = biaser.Add(v.position, float3(0.5f, 0.5f, 0.5f));
    float combined = biaser.Combine(v.weight) + biaser.Combine(v.weight, 0.5f);
    o.position = float4(vector, 1.0f);
    o.color = float4(vector.x, vector.y, vector.z, scalar + combined);
    return o;
}
