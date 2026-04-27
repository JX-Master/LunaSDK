#include <cppsl/core.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float4 color;
    [[cppsl::location(1)]] float weight;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 xy;
    [[cppsl::location(1)]] float3 xyz;
    [[cppsl::location(2)]] float3 yzw;
    [[cppsl::location(3)]] float4 rgba;
    [[cppsl::location(4)]] float4 xgba;
    [[cppsl::location(5)]] float4 xyzw;
    [[cppsl::location(6)]] float4 splat;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput input)
{
    VSOutput output;
    output.xy = float2{input.color.x, input.color.y};
    output.xyz = float3{input.color.xy, input.color.z};
    output.yzw = float3{input.color.y, input.color.zw};
    output.rgba = float4{input.color.rgb, input.weight};
    output.xgba = float4{input.color.x, input.color.gba};
    output.xyzw = float4{input.color.xy, input.color.zw};
    output.splat = float4{input.weight};
    output.position = output.rgba;
    return output;
}
