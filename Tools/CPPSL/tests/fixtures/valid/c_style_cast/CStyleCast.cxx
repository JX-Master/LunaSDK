#include <cppsl/core.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float value;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput vertex_data)
{
    uint unsigned_value = (uint)vertex_data.value;
    int signed_value = (int)unsigned_value;
    float float_value = (float)signed_value;
    VSOutput result;
    result.position = float4{float_value, 0.0f, 0.0f, 1.0f};
    return result;
}
