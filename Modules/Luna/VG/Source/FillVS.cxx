#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct TransformParams
{
    float4x4 transform;
    float4 clip_rect;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    TransformParams g_cbuffer;

    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const float* g_commands;

    [[cppsl::binding(2)]]
    Texture2D<float4> g_tex;

    [[cppsl::binding(3)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VSIn
{
    [[cppsl::location(0)]] float2 position;
    [[cppsl::location(1)]] float2 shapecoord;
    [[cppsl::location(2)]] float2 texcoord;
    [[cppsl::location(3)]] uint begin_command_offset;
    [[cppsl::location(4)]] uint num_commands;
    [[cppsl::location(5)]] float4 color;
};

struct VSOut
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 position_2d;
    [[cppsl::location(2)]] float2 shapecoord;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] uint begin_command_offset;
    [[cppsl::location(5)]] uint num_commands;
    [[cppsl::location(6)]] float4 color;
};

[[cppsl::vertex]]
VSOut vs_main(VSIn v)
{
    float4 pos;
    pos = float4{v.position.x, v.position.y, 0.0f, 1.0f};
    pos = mul(g_set0.g_cbuffer.transform, pos);

    VSOut o;
    o.position = pos;
    o.position_2d = v.position;
    o.shapecoord = v.shapecoord;
    o.texcoord = v.texcoord;
    o.color = v.color;
    o.begin_command_offset = v.begin_command_offset;
    o.num_commands = v.num_commands;
    return o;
}
