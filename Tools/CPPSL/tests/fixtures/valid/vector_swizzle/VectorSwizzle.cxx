#include <cppsl/core.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float4 color;
    [[cppsl::location(1)]] float3 normal;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float3 rgb;
    [[cppsl::location(1)]] float2 zw;
    [[cppsl::location(2)]] float4 reversed;
    [[cppsl::location(3)]] float4 repeated;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    float3 xyz = v.color.xyz;
    float3 rgb = v.color.rgb;
    float3 zwx = v.color.zwx;
    float2 zw = v.color.zw;
    float4 reversed = v.color.wzyx;
    float4 repeated = v.normal.xyyz;

    VSOutput o;
    o.position = float4{xyz.x, rgb.y, zwx.z, 1.0f};
    o.rgb = rgb;
    o.zw = zw;
    o.reversed = reversed;
    o.repeated = repeated;
    return o;
}
