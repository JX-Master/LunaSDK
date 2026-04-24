#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

struct Item
{
    float value;
};

struct FrameSet
{
    [[cppsl::binding(0), cppsl::cbuffer]]
    Camera camera;

    [[cppsl::binding(1), cppsl::structured_buffer]]
    const Item* items;

    [[cppsl::binding(2), cppsl::rwstructured_buffer]]
    float* output_values;

    [[cppsl::binding(3)]]
    Texture2D<float4> color_texture;

    [[cppsl::binding(4)]]
    SamplerState color_sampler;
};

[[cppsl::desc_set(0)]]
FrameSet frame;

struct VSInput
{
    [[cppsl::location(0)]]
    float3 position;
    [[cppsl::location(1)]]
    float2 texcoord;
};

struct VSOutput
{
    [[cppsl::position]]
    float4 position;
    [[cppsl::location(1)]]
    float4 color;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput input)
{
    VSOutput output;
    output.position = mul(frame.camera.world_to_proj, float4{input.position.x, input.position.y, input.position.z, 1.0f});
    output.color = frame.color_texture.Sample(frame.color_sampler, input.texcoord) * frame.items[0].value;
    frame.output_values[0] = output.color.x;
    return output;
}
