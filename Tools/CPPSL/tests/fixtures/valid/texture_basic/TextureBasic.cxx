#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct TextureSet
{
    [[cppsl::binding(0)]]
    Texture2D<float4> color_texture;

    [[cppsl::binding(1)]]
    SamplerState linear_sampler;
};

[[cppsl::desc_set(0)]]
TextureSet textures;

struct PSInput
{
    [[cppsl::location(0)]] float2 uv;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PSOutput main_ps(PSInput v)
{
    PSOutput o;
    o.color = textures.color_texture.Sample(textures.linear_sampler, v.uv);
    return o;
}
