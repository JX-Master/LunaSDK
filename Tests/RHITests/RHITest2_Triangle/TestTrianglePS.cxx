#include <cppsl/core.hxx>

using namespace cppsl;

struct PS_INPUT
{
    [[cppsl::location(1)]] float4 col;
};

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = input.col;
    return output;
}
