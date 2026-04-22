#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::set(0), cppsl::binding(0)]]
Texture2D<float4> color_texture;

[[cppsl::set(0), cppsl::binding(1)]]
SamplerState linear_sampler;

struct PSInput
{
    [[cppsl::location(0)]] float2 uv;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PSOutput main_ps(PSInput input)
{
    PSOutput output;
    output.color = color_texture.Sample(linear_sampler, input.uv);
    return output;
}
