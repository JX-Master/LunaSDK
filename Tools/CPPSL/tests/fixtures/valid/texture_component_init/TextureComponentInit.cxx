#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct TextureSet
{
    [[cppsl::binding(0)]]
    Texture2D<float4> src_tex;

    [[cppsl::binding(1)]]
    SamplerState linear_sampler;
};

[[cppsl::desc_set(0)]]
TextureSet textures;

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 uv;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::pixel]]
PSOutput main_ps(PSInput input)
{
    float sampled = textures.src_tex.Sample(textures.linear_sampler, input.uv).x;
    PSOutput output;
    output.color = float4{sampled, sampled, sampled, 1.0f};
    return output;
}
