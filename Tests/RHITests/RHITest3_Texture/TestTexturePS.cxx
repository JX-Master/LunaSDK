#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::desc_set(0), cppsl::binding(0)]]
Texture2D<float4> texture0;

[[cppsl::desc_set(0), cppsl::binding(1)]]
SamplerState sampler0;

struct PS_INPUT
{
    [[cppsl::location(1)]] float2 uv;
};

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = clamp(
        texture0.Sample(sampler0, input.uv),
        float4{0.0f, 0.0f, 0.0f, 0.0f},
        float4{1.0f, 1.0f, 1.0f, 1.0f});
    return output;
}
