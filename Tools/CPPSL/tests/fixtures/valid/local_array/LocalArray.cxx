#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float weight;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
};

float sum_values(float base)
{
    float values[4];
    values[0] = base;
    values[1] = base + 1.0f;
    values[2] = base + 2.0f;
    values[3] = base + 3.0f;
    float result = 0.0f;
    int i = 0;
    while(i < 4)
    {
        result += values[i];
        i += 1;
    }
    return result;
}

[[cppsl::vertex]]
VSOutput main_vs(VSInput vertex_data)
{
    VSOutput result;
    result.position = float4{sum_values(vertex_data.weight), 0.0f, 0.0f, 1.0f};
    return result;
}
