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
VSOutput main_vs(VSInput v)
{
    VSOutput o;
    o.xy = float2{v.color.x, v.color.y};
    o.xyz = float3{v.color.xy, v.color.z};
    o.yzw = float3{v.color.y, v.color.zw};
    o.rgba = float4{v.color.rgb, v.weight};
    o.xgba = float4{v.color.x, v.color.gba};
    o.xyzw = float4{v.color.xy, v.color.zw};
    o.splat = float4{v.weight};
    o.position = o.rgba;
    return o;
}
