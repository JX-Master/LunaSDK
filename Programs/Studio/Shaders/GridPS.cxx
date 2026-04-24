#include <cppsl/core.hxx>

using namespace cppsl;

struct PS_INPUT
{
    [[cppsl::position]] float4 pos;
};

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = float4{1.0f, 1.0f, 1.0f, 1.0f};
    return output;
}
