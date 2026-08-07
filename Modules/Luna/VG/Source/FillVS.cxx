#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct TransformParams
{
    float4x4 transform;
};

struct ShapeState
{
    float4x4 transform;
    float4 clip_rect;
    float4 rounded_clip_rect;
    float4 rounded_clip_radii;
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

    [[cppsl::structured_buffer, cppsl::binding(4)]]
    const ShapeState* g_states;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VSIn
{
    [[cppsl::location(0)]] float2 position;
    [[cppsl::location(1)]] float4 position_bounds;
    [[cppsl::location(2)]] float4 shapecoord_bounds;
    [[cppsl::location(3)]] float4 texcoord_bounds;
    [[cppsl::location(4)]] uint4 command_range_and_state;
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
    [[cppsl::location(7)]] float4 clip_rect;
    [[cppsl::location(8)]] float4 rounded_clip_rect;
    [[cppsl::location(9)]] float4 rounded_clip_radii;
};

[[cppsl::vertex]]
VSOut vs_main(VSIn v)
{
    float2 position = v.position_bounds.xy + v.position *
        (v.position_bounds.zw - v.position_bounds.xy);
    float4 pos;
    pos = float4{position, 0.0f, 1.0f};
    pos = mul(g_set0.g_cbuffer.transform, pos);
    ShapeState state = g_set0.g_states[v.command_range_and_state.z];
    pos = mul(state.transform, pos);

    VSOut o;
    o.position = pos;
    o.position_2d = position;
    o.shapecoord = v.shapecoord_bounds.xy + v.position *
        (v.shapecoord_bounds.zw - v.shapecoord_bounds.xy);
    o.texcoord = v.texcoord_bounds.xy + v.position *
        (v.texcoord_bounds.zw - v.texcoord_bounds.xy);
    o.color = v.color;
    o.begin_command_offset = v.command_range_and_state.x;
    o.num_commands = v.command_range_and_state.y;
    o.clip_rect = state.clip_rect;
    o.rounded_clip_rect = state.rounded_clip_rect;
    o.rounded_clip_radii = state.rounded_clip_radii;
    return o;
}
