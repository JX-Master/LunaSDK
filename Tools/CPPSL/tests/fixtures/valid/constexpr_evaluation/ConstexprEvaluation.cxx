#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

constexpr int kSteps = 2 + 2;
constexpr float kBias = 0.25f * 2.0f;
constexpr bool kUseBias = kSteps == 4;

constexpr float StaticBias()
{
    return kBias + 0.125f;
}

struct VSInput
{
    [[cppsl::location(0)]]
    float weight;
};

struct VSOutput
{
    [[cppsl::position]]
    float4 position;
    [[cppsl::location(0)]]
    float4 color;
};

float ApplyConstexpr(float value)
{
    constexpr float local_scale = StaticBias() * 2.0f;
    if constexpr (kUseBias)
    {
        return value + local_scale + float(kSteps);
    }
    else
    {
        return value - 999.0f;
    }
}

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    VSOutput o;
    o.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    o.color = float4(ApplyConstexpr(v.weight), float(kSteps), StaticBias(), 1.0f);
    return o;
}
