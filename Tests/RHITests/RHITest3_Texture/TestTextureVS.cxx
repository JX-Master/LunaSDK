#include <cppsl/core.hxx>

using namespace cppsl;

struct VS_INPUT
{
    [[cppsl::location(0)]] float2 pos;
    [[cppsl::location(1)]] float2 uv;
};

struct PS_INPUT
{
    [[cppsl::position]] float4 pos;
    [[cppsl::location(1)]] float2 uv;
};

[[cppsl::vertex]]
PS_INPUT vs_main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4{input.pos.x, input.pos.y, 0.0f, 1.0f};
    output.uv = input.uv;
    return output;
}
