#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float4 color;
    [[cppsl::location(1)]] float exposure;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float4 color;
};

struct ToneMapper
{
    float exposure;

    float3 Apply(float3 color) const
    {
        return color * exposure + float3{0.25f, 0.25f, 0.25f};
    }
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    ToneMapper mapper;
    mapper.exposure = v.exposure;

    VSOutput o;
    o.position = float4{0.0f, 0.0f, 0.0f, 1.0f};
    o.color = float4{mapper.Apply(v.color.rgb), 1.0f};
    return o;
}
