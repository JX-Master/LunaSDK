#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct SDFFrameParams
{
    float4 screen_params;
};

struct SDFState
{
    float4 clip_rect;
    float4 rounded_clip_rect;
    float4 rounded_clip_radii;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    SDFFrameParams frame;

    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const float* shape_floats;

    [[cppsl::structured_buffer, cppsl::binding(2)]]
    const float* color_floats;

    [[cppsl::structured_buffer, cppsl::binding(3)]]
    const SDFState* states;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VSInput
{
    [[cppsl::location(0)]] float2 position;
    [[cppsl::location(1)]] float4 draw_rect;
    [[cppsl::location(2)]] float2 evaluation_origin;
    [[cppsl::location(3)]] uint4 program_data;
};

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 screen_position;
    [[cppsl::location(1)]] float2 evaluation_origin;
    [[cppsl::location(2)]] float4 clip_rect;
    [[cppsl::location(3)]] float4 rounded_clip_rect;
    [[cppsl::location(4)]] float4 rounded_clip_radii;
    [[cppsl::location(5)]] uint4 program_data;
};

[[cppsl::vertex]]
PSInput vs_main(VSInput vertex_data)
{
    float2 draw_origin = float2{vertex_data.draw_rect.x, vertex_data.draw_rect.y};
    float2 draw_size = float2{vertex_data.draw_rect.z, vertex_data.draw_rect.w};
    float2 screen_position = draw_origin + vertex_data.position * draw_size;
    float2 clip_position;
    clip_position.x = screen_position.x / g_set0.frame.screen_params.x * 2.0f - 1.0f;
    clip_position.y = 1.0f - screen_position.y / g_set0.frame.screen_params.y * 2.0f;
    PSInput result;
    result.position = float4{clip_position, 0.0f, 1.0f};
    result.screen_position = screen_position;
    result.evaluation_origin = vertex_data.evaluation_origin;
    SDFState state = g_set0.states[vertex_data.program_data.w];
    result.clip_rect = state.clip_rect;
    result.rounded_clip_rect = state.rounded_clip_rect;
    result.rounded_clip_radii = state.rounded_clip_radii;
    result.program_data = vertex_data.program_data;
    return result;
}
