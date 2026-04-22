#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/resource.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

[[cppsl::set(0), cppsl::binding(0)]]
ConstantBuffer<Camera> camera;

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
    output.position = mul(camera->world_to_proj, float4{input.position.x, input.position.y, input.position.z, 1.0f});
    output.texcoord = input.texcoord;
    return output;
}
