#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct TransformParams
{
    float4x4 transform;
    float4 clip_rect;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
TransformParams g_cbuffer;

[[cppsl::structured_buffer, cppsl::desc_set(0), cppsl::binding(1)]]
const float* g_commands;

[[cppsl::desc_set(0), cppsl::binding(2)]]
Texture2D<float4> g_tex;

[[cppsl::desc_set(0), cppsl::binding(3)]]
SamplerState g_sampler;

struct PSIn
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 position_2d;
    [[cppsl::location(2)]] float2 shapecoord;
    [[cppsl::location(3)]] float2 texcoord;
    [[cppsl::location(4)]] uint begin_command_offset;
    [[cppsl::location(5)]] uint num_commands;
    [[cppsl::location(6)]] float4 color;
};

struct PSOut
{
    [[cppsl::location(0)]] float4 color;
};

float line_test_x_axis(float2 v0, float2 v1, float2 pixels_per_unit)
{
    if (max(v0.x, v1.x) * pixels_per_unit.x < -0.5f) return 0.0f;
    int sign = ((v0.y > 0.0f) ? 1 : 0) - ((v1.y > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float xt = (v1.y * v0.x - v0.y * v1.x) / (v1.y - v0.y);
    return sign * saturate(xt * pixels_per_unit.x + 0.5f);
}

float line_test_y_axis(float2 v0, float2 v1, float2 pixels_per_unit)
{
    if (max(v0.y, v1.y) * pixels_per_unit.y < -0.5f) return 0.0f;
    int sign = ((v1.x > 0.0f) ? 1 : 0) - ((v0.x > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float yt = (v1.x * v0.y - v0.x * v1.y) / (v1.x - v0.x);
    return sign * saturate(yt * pixels_per_unit.y + 0.5f);
}

float2 quad_curve_solve_x_axis(float2 v0, float2 v1, float2 v2)
{
    float2 a = v0 - 2.0f * v1 + v2;
    float2 b = v0 - v1;
    float c = v0.y;
    float ra = 1.0f / a.y;
    float rb = 0.5f / b.y;
    float delta = sqrt(max(b.y * b.y - a.y * c, 0.0f));
    float2 t = float2{(b.y - delta) * ra, (b.y + delta) * ra};
    if (abs(a.y) < 0.0001220703125f) t = float2{c * rb, c * rb};
    return (a.x * t - b.x * 2.0f) * t + v0.x;
}

float2 quad_curve_solve_y_axis(float2 v0, float2 v1, float2 v2)
{
    float2 a = v0 - 2.0f * v1 + v2;
    float2 b = v0 - v1;
    float c = v0.x;
    float ra = 1.0f / a.x;
    float rb = 0.5f / b.x;
    float delta = sqrt(max(b.x * b.x - a.x * c, 0.0f));
    float2 t = float2{(b.x - delta) * ra, (b.x + delta) * ra};
    if (abs(a.x) < 0.0001220703125f) t = float2{c * rb, c * rb};
    return (a.y * t - b.y * 2.0f) * t + v0.y;
}

int get_curve_root_flags(float v0, float v1, float v2)
{
    int shift = ((v0 > 0.0f) ? 2 : 0) + ((v1 > 0.0f) ? 4 : 0) + ((v2 > 0.0f) ? 8 : 0);
    return (0x2E74 >> shift) & 0x03;
}

float curve_test_x_axis(float2 v0, float2 v1, float2 v2, float2 pixels_per_unit)
{
    if (max(max(v0.x, v1.x), v2.x) * pixels_per_unit.x < -0.5f) return 0.0f;
    int flags = get_curve_root_flags(v0.y, v1.y, v2.y);
    if (flags == 0) return 0.0f;
    float2 x1x2 = quad_curve_solve_x_axis(v0, v1, v2) * pixels_per_unit.x;
    float ret = 0.0f;
    if ((flags & 0x01) != 0)
    {
        ret += saturate(x1x2.x + 0.5f);
    }
    if ((flags & 0x02) != 0)
    {
        ret -= saturate(x1x2.y + 0.5f);
    }
    return ret;
}

float curve_test_y_axis(float2 v0, float2 v1, float2 v2, float2 pixels_per_unit)
{
    if (max(max(v0.y, v1.y), v2.y) * pixels_per_unit.y < -0.5f) return 0.0f;
    int flags = get_curve_root_flags(v0.x, v1.x, v2.x);
    if (flags == 0) return 0.0f;
    float2 y1y2 = quad_curve_solve_y_axis(v0, v1, v2) * pixels_per_unit.y;
    float ret = 0.0f;
    if ((flags & 0x01) != 0)
    {
        ret -= saturate(y1y2.x + 0.5f);
    }
    if ((flags & 0x02) != 0)
    {
        ret += saturate(y1y2.y + 0.5f);
    }
    return ret;
}

float2 circle_get_point(float2 center, float radius, float angle)
{
    angle = angle * 0.0174532925222222f;
    return center + radius * float2{cos(angle), sin(angle)};
}

float2 ellipse_get_point(float2 center, float2 radius, float angle)
{
    angle = angle * 0.0174532925222222f;
    return center + radius * float2{cos(angle), sin(angle)};
}

float circle_test_x_axis(float2 v0, float2 v1, float2 center, float radius, bool choose_first, float2 pixels_per_unit)
{
    if (max(v0.x, v1.x) * pixels_per_unit.x < -0.5f) return 0.0f;
    int sign = ((v0.y > 0.0f) ? 1 : 0) - ((v1.y > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float delta = sqrt(radius * radius - center.y * center.y);
    float xt = choose_first ? center.x - delta : center.x + delta;
    return sign * saturate(xt * pixels_per_unit.x + 0.5f);
}

float circle_test_y_axis(float2 v0, float2 v1, float2 center, float radius, bool choose_first, float2 pixels_per_unit)
{
    if (max(v0.y, v1.y) * pixels_per_unit.y < -0.5f) return 0.0f;
    int sign = ((v1.x > 0.0f) ? 1 : 0) - ((v0.x > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float delta = sqrt(radius * radius - center.x * center.x);
    float yt = choose_first ? center.y - delta : center.y + delta;
    return sign * saturate(yt * pixels_per_unit.y + 0.5f);
}

float ellipse_test_x_axis(float2 v0, float2 v1, float2 center, float2 radius, bool choose_first, float2 pixels_per_unit)
{
    if (max(v0.x, v1.x) * pixels_per_unit.x < -0.5f) return 0.0f;
    int sign = ((v0.y > 0.0f) ? 1 : 0) - ((v1.y > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float normy = center.y / radius.y;
    float delta = sqrt(1.0f - normy * normy) * radius.x;
    float xt = choose_first ? center.x - delta : center.x + delta;
    return sign * saturate(xt * pixels_per_unit.x + 0.5f);
}

float ellipse_test_y_axis(float2 v0, float2 v1, float2 center, float2 radius, bool choose_first, float2 pixels_per_unit)
{
    if (max(v0.y, v1.y) * pixels_per_unit.y < -0.5f) return 0.0f;
    int sign = ((v1.x > 0.0f) ? 1 : 0) - ((v0.x > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float normx = center.x / radius.x;
    float delta = sqrt(1.0f - normx * normx) * radius.y;
    float yt = choose_first ? center.y - delta : center.y + delta;
    return sign * saturate(yt * pixels_per_unit.y + 0.5f);
}

float clip_rect_test(float2 pos, float4 clip_rect, float2 pixels_per_unit)
{
    float4 rect = float4{clip_rect.x, clip_rect.y, clip_rect.x + clip_rect.z, clip_rect.y + clip_rect.w};
    rect -= float4{pos.x, pos.y, pos.x, pos.y};
    if (rect.x > 0.0f || rect.y > 0.0f || rect.z < 0.0f || rect.w < 0.0f)
    {
        return 0.0f;
    }
    float2 dist0 = float2{abs(rect.x), abs(rect.y)};
    float2 dist1 = float2{abs(rect.z), abs(rect.w)};
    float2 dist = min(dist0, dist1);
    dist = saturate(dist * pixels_per_unit);
    return min(dist.x, dist.y);
}

[[cppsl::fragment]]
PSOut ps_main(PSIn v)
{
    float2 units_per_pixel = fwidth(v.shapecoord);
    float2 pixels_per_unit = 1.0f / units_per_pixel;
    float coverage_x = 0.0f;
    float coverage_y = 0.0f;
    uint i = v.begin_command_offset;
    uint command_end = i + v.num_commands;
    float2 last_point = float2{0.0f, 0.0f};

    while (i < command_end)
    {
        float command = g_commands[i];
        if (command == 1.0f)
        {
            last_point = float2{g_commands[i + 1], g_commands[i + 2]};
            i += 3;
        }
        else if (command == 2.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            last_point = float2{g_commands[i + 1], g_commands[i + 2]};
            float2 v1 = last_point - v.shapecoord;
            coverage_x += line_test_x_axis(v0, v1, pixels_per_unit);
            coverage_y += line_test_y_axis(v0, v1, pixels_per_unit);
            i += 3;
        }
        else if (command == 3.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            float2 v1 = float2{g_commands[i + 1], g_commands[i + 2]} - v.shapecoord;
            last_point = float2{g_commands[i + 3], g_commands[i + 4]};
            float2 v2 = last_point - v.shapecoord;
            coverage_x += curve_test_x_axis(v0, v1, v2, pixels_per_unit);
            coverage_y += curve_test_y_axis(v0, v1, v2, pixels_per_unit);
            i += 5;
        }
        else if (command >= 4.0f && command <= 7.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            float radius = g_commands[i + 1];
            float begin = g_commands[i + 2];
            float command_arc_end = g_commands[i + 3];
            float2 center = circle_get_point(last_point, radius, 180.0f + begin);
            last_point = circle_get_point(center, radius, command_arc_end);
            float2 v1 = last_point - v.shapecoord;
            center = center - v.shapecoord;
            coverage_x += circle_test_x_axis(v0, v1, center, radius, (command == 5.0f) || (command == 6.0f), pixels_per_unit);
            coverage_y += circle_test_y_axis(v0, v1, center, radius, (command == 6.0f) || (command == 7.0f), pixels_per_unit);
            i += 4;
        }
        else if (command >= 8.0f && command <= 11.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            float rx = g_commands[i + 1];
            float ry = g_commands[i + 2];
            float2 radius = float2{rx, ry};
            float begin = g_commands[i + 3];
            float command_arc_end = g_commands[i + 4];
            float2 center = ellipse_get_point(last_point, radius, 180.0f + begin);
            last_point = ellipse_get_point(center, radius, command_arc_end);
            float2 v1 = last_point - v.shapecoord;
            center = center - v.shapecoord;
            coverage_x += ellipse_test_x_axis(v0, v1, center, radius, (command == 9.0f) || (command == 10.0f), pixels_per_unit);
            coverage_y += ellipse_test_y_axis(v0, v1, center, radius, (command == 10.0f) || (command == 11.0f), pixels_per_unit);
            i += 5;
        }
    }

    float weight_x = 1.0f - abs(abs(coverage_x) * 2.0f - 1.0f);
    float weight_y = 1.0f - abs(abs(coverage_y) * 2.0f - 1.0f);
    float coverage = max(abs(coverage_x * weight_x + coverage_y * weight_y) / max(weight_x + weight_y, 0.0001220703125f), min(abs(coverage_x), abs(coverage_y)));
    if (any(g_cbuffer.clip_rect != float4{0.0f, 0.0f, 0.0f, 0.0f}))
    {
        float2 pos_units_per_pixel = fwidth(v.position_2d);
        float2 pixels_per_pos_unit = 1.0f / pos_units_per_pixel;
        coverage *= clip_rect_test(v.position_2d, g_cbuffer.clip_rect, pixels_per_pos_unit);
    }

    PSOut output;
    float4 col = g_tex.Sample(g_sampler, v.texcoord);
    col *= v.color;
    col.w *= coverage;
    output.color = col;
    return output;
}
