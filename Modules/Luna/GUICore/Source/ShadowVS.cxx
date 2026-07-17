#include <cppsl/core.hxx>

using namespace cppsl;

struct ShadowParams
{
    float4 draw_rect;
    float4 source_rect;
    float4 clip_rect;
    float4 color;
    float4 shadow_params;
    float4 screen_params;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    ShadowParams params;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct VSInput
{
    [[cppsl::location(0)]] float2 position;
};

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 screen_position;
};

[[cppsl::vertex]]
PSInput vs_main(VSInput vertex_data)
{
    float2 draw_origin = float2{g_set0.params.draw_rect.x, g_set0.params.draw_rect.y};
    float2 draw_size = float2{g_set0.params.draw_rect.z, g_set0.params.draw_rect.w};
    float2 screen_position;
    screen_position.x = draw_origin.x + vertex_data.position.x * draw_size.x;
    screen_position.y = draw_origin.y + vertex_data.position.y * draw_size.y;
    float2 clip_position;
    clip_position.x = screen_position.x / g_set0.params.screen_params.x * 2.0f - 1.0f;
    clip_position.y = 1.0f - screen_position.y / g_set0.params.screen_params.y * 2.0f;
    PSInput result;
    result.position = float4{clip_position, 0.0f, 1.0f};
    result.screen_position = screen_position;
    return result;
}
