#include <cppsl/core.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float value;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float value;
};

struct Accumulator
{
    float value;

    void Add(float delta)
    {
        value += delta;
    }

    float Read() const
    {
        return value;
    }
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    Accumulator accumulator;
    accumulator.value = 1.0f;
    accumulator.Add(v.value);

    VSOutput o;
    o.position = float4{0.0f, 0.0f, 0.0f, 1.0f};
    o.value = accumulator.Read();
    return o;
}
