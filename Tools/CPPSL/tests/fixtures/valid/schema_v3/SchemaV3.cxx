#include <cppsl/core.hxx>

constexpr int kBias = 3;

float AddDefault(float value, float bias = 1.0f)
{
    return value + bias;
}

template<typename T>
T Identity(T value)
{
    return value;
}

struct Accumulator
{
    float bias;

    float Apply(float value) const
    {
        return value + bias;
    }
};

float TouchSchemaV3(float value)
{
    Accumulator accumulator;
    accumulator.bias = float(kBias);
    return accumulator.Apply(Identity(AddDefault(value)));
}
