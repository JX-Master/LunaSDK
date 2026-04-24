#include "WireframeDescSet.hxx"

struct PS_INPUT
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float3 normal;
    [[cppsl::location(2)]] float3 tangent;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] float4 color;
    [[cppsl::location(5)]] float3 world_position;
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
