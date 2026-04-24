#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/resource.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

struct FrameSet
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera camera;
};

[[cppsl::desc_set(0)]]
FrameSet frame;

struct VSInput
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 texcoord;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput input)
{
    VSOutput output;
    output.position = mul(frame.camera.world_to_proj, float4{input.position.x, input.position.y, input.position.z, 1.0f});
    output.texcoord = input.texcoord;
    return output;
}
