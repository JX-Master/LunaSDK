#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct Bad
{
    float value;

    float operator+(float rhs) const
    {
        return value + rhs;
    }
};

struct VSOutput
{
    [[cppsl::position]]
    float4 position;
};

[[cppsl::vertex]]
VSOutput main_vs()
{
    VSOutput o;
    o.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    return o;
}
