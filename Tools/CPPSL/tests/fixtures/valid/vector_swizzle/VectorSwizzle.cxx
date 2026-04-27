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
VSOutput main_vs(VSInput input)
{
    float3 xyz = input.color.xyz;
    float3 rgb = input.color.rgb;
    float3 zwx = input.color.zwx;
    float2 zw = input.color.zw;
    float4 reversed = input.color.wzyx;
    float4 repeated = input.normal.xyyz;

    VSOutput output;
    output.position = float4{xyz.x, rgb.y, zwx.z, 1.0f};
    output.rgb = rgb;
    output.zw = zw;
    output.reversed = reversed;
    output.repeated = repeated;
    return output;
}
