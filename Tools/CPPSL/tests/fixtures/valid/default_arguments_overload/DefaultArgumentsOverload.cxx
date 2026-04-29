#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

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

float MixValue(float value, float bias = 0.25f)
{
    return value + bias;
}

float2 MixValue(float2 value, float bias)
{
    return value + float2(bias, bias);
}

struct Mixer
{
    float bias;

    float Apply(float value) const
    {
        return value + bias;
    }

    float3 Apply(float3 value) const
    {
        return value + float3(bias, bias, bias);
    }
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    VSOutput o;
    Mixer mixer;
    mixer.bias = 0.125f;
    float scalar = mixer.Apply(MixValue(v.weight));
    float3 vector = mixer.Apply(v.position);
    float2 pair = MixValue(float2(v.weight, scalar), 0.5f);
    o.position = float4(v.position, 1.0f);
    o.color = float4(vector + float3(pair.x, pair.y, scalar), 1.0f);
    return o;
}
