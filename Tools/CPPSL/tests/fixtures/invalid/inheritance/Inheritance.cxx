#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct Base
{
    float value;
};

struct Derived : Base
{
    float extra;
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
