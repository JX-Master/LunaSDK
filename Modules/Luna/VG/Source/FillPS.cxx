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

struct RootSet
{
    float r0;
    float r1;
    float r2;
    int count;
};

RootSet make_root_set()
{
    RootSet ret;
    ret.r0 = 0.0f;
    ret.r1 = 0.0f;
    ret.r2 = 0.0f;
    ret.count = 0;
    return ret;
}

float root_at(RootSet roots, int index)
{
    if (index == 0) return roots.r0;
    if (index == 1) return roots.r1;
    return roots.r2;
}

RootSet set_root_at(RootSet roots, int index, float value)
{
    if (index == 0) roots.r0 = value;
    else if (index == 1) roots.r1 = value;
    else roots.r2 = value;
    return roots;
}

RootSet sort_roots(RootSet roots)
{
    if (roots.count > 1 && roots.r1 < roots.r0)
    {
        float t = roots.r0;
        roots.r0 = roots.r1;
        roots.r1 = t;
    }
    if (roots.count > 2 && roots.r2 < roots.r1)
    {
        float t = roots.r1;
        roots.r1 = roots.r2;
        roots.r2 = t;
    }
    if (roots.count > 1 && roots.r1 < roots.r0)
    {
        float t = roots.r0;
        roots.r0 = roots.r1;
        roots.r1 = t;
    }
    return roots;
}

RootSet add_root(RootSet roots, float root)
{
    if (root < -0.0001220703125f || root > 1.0001220703125f) return roots;
    root = clamp(root, 0.0f, 1.0f);
    if (roots.count > 0 && abs(root - roots.r0) < 0.000244140625f) return roots;
    if (roots.count > 1 && abs(root - roots.r1) < 0.000244140625f) return roots;
    if (roots.count > 2 && abs(root - roots.r2) < 0.000244140625f) return roots;
    if (roots.count >= 3) return roots;
    roots = set_root_at(roots, roots.count, root);
    roots.count += 1;
    return sort_roots(roots);
}

RootSet solve_linear_roots(float a, float b)
{
    RootSet roots = make_root_set();
    if (abs(a) < 0.0001220703125f) return roots;
    return add_root(roots, -b / a);
}

RootSet solve_quadratic_roots(float a, float b, float c)
{
    if (abs(a) < 0.0001220703125f)
    {
        return solve_linear_roots(b, c);
    }
    RootSet roots = make_root_set();
    float delta = b * b - 4.0f * a * c;
    if (delta < -0.0001220703125f) return roots;
    if (abs(delta) <= 0.0001220703125f)
    {
        return add_root(roots, -b / (2.0f * a));
    }
    float sqrt_delta = sqrt(delta);
    roots = add_root(roots, (-b - sqrt_delta) / (2.0f * a));
    roots = add_root(roots, (-b + sqrt_delta) / (2.0f * a));
    return roots;
}

float cubic_eval(float v0, float v1, float v2, float v3, float t)
{
    float u = 1.0f - t;
    return u * u * u * v0 + 3.0f * u * u * t * v1 + 3.0f * u * t * t * v2 + t * t * t * v3;
}

RootSet cubic_derivative_roots(float v0, float v1, float v2, float v3)
{
    float a = -v0 + 3.0f * v1 - 3.0f * v2 + v3;
    float b = 3.0f * v0 - 6.0f * v1 + 3.0f * v2;
    float c = -3.0f * v0 + 3.0f * v1;
    return solve_quadratic_roots(3.0f * a, 2.0f * b, c);
}

float cubic_find_root(float v0, float v1, float v2, float v3, float lo, float hi)
{
    float flo = cubic_eval(v0, v1, v2, v3, lo);
    if (abs(flo) <= 0.0001220703125f) return lo;
    float fhi = cubic_eval(v0, v1, v2, v3, hi);
    if (abs(fhi) <= 0.0001220703125f) return hi;
    float a = lo;
    float b = hi;
    float fa = flo;
    int i = 0;
    while (i < 16)
    {
        float m = (a + b) * 0.5f;
        float fm = cubic_eval(v0, v1, v2, v3, m);
        if ((fa > 0.0f) == (fm > 0.0f))
        {
            a = m;
            fa = fm;
        }
        else
        {
            b = m;
        }
        i += 1;
    }
    return (a + b) * 0.5f;
}

float cubic_interval_test_x_axis(float2 v0, float2 v1, float2 v2, float2 v3, float lo, float hi, float2 pixels_per_unit)
{
    if (hi - lo < 0.0001220703125f) return 0.0f;
    float flo = cubic_eval(v0.y, v1.y, v2.y, v3.y, lo);
    float fhi = cubic_eval(v0.y, v1.y, v2.y, v3.y, hi);
    int sign = ((flo > 0.0f) ? 1 : 0) - ((fhi > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float t = cubic_find_root(v0.y, v1.y, v2.y, v3.y, lo, hi);
    float x = cubic_eval(v0.x, v1.x, v2.x, v3.x, t) * pixels_per_unit.x;
    return sign * saturate(x + 0.5f);
}

float cubic_interval_test_y_axis(float2 v0, float2 v1, float2 v2, float2 v3, float lo, float hi, float2 pixels_per_unit)
{
    if (hi - lo < 0.0001220703125f) return 0.0f;
    float flo = cubic_eval(v0.x, v1.x, v2.x, v3.x, lo);
    float fhi = cubic_eval(v0.x, v1.x, v2.x, v3.x, hi);
    int sign = ((fhi > 0.0f) ? 1 : 0) - ((flo > 0.0f) ? 1 : 0);
    if (sign == 0) return 0.0f;
    float t = cubic_find_root(v0.x, v1.x, v2.x, v3.x, lo, hi);
    float y = cubic_eval(v0.y, v1.y, v2.y, v3.y, t) * pixels_per_unit.y;
    return sign * saturate(y + 0.5f);
}

float cubic_curve_test_x_axis(float2 v0, float2 v1, float2 v2, float2 v3, float2 pixels_per_unit)
{
    if (max(max(v0.x, v1.x), max(v2.x, v3.x)) * pixels_per_unit.x < -0.5f) return 0.0f;
    float min_y = min(min(v0.y, v1.y), min(v2.y, v3.y));
    float max_y = max(max(v0.y, v1.y), max(v2.y, v3.y));
    if (min_y > 0.0f || max_y < 0.0f) return 0.0f;

    RootSet roots = cubic_derivative_roots(v0.y, v1.y, v2.y, v3.y);
    float ret = 0.0f;
    float lo = 0.0f;
    int i = 0;
    while (i < roots.count)
    {
        float hi = root_at(roots, i);
        ret += cubic_interval_test_x_axis(v0, v1, v2, v3, lo, hi, pixels_per_unit);
        lo = hi;
        i += 1;
    }
    ret += cubic_interval_test_x_axis(v0, v1, v2, v3, lo, 1.0f, pixels_per_unit);
    return ret;
}

float cubic_curve_test_y_axis(float2 v0, float2 v1, float2 v2, float2 v3, float2 pixels_per_unit)
{
    if (max(max(v0.y, v1.y), max(v2.y, v3.y)) * pixels_per_unit.y < -0.5f) return 0.0f;
    float min_x = min(min(v0.x, v1.x), min(v2.x, v3.x));
    float max_x = max(max(v0.x, v1.x), max(v2.x, v3.x));
    if (min_x > 0.0f || max_x < 0.0f) return 0.0f;

    RootSet roots = cubic_derivative_roots(v0.x, v1.x, v2.x, v3.x);
    float ret = 0.0f;
    float lo = 0.0f;
    int i = 0;
    while (i < roots.count)
    {
        float hi = root_at(roots, i);
        ret += cubic_interval_test_y_axis(v0, v1, v2, v3, lo, hi, pixels_per_unit);
        lo = hi;
        i += 1;
    }
    ret += cubic_interval_test_y_axis(v0, v1, v2, v3, lo, 1.0f, pixels_per_unit);
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

float clip_rect_test(float2 pos, float4 clip_rect)
{
    float4 rect = float4{clip_rect.xy, clip_rect.xy + clip_rect.zw};
    rect -= float4{pos, pos};
    if (rect.x > 0.0f || rect.y > 0.0f || rect.z < 0.0f || rect.w < 0.0f)
    {
        return 0.0f;
    }
    return 1.0f;
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
        float command = g_set0.g_commands[i];
        if (command == 1.0f)
        {
            last_point = float2{g_set0.g_commands[i + 1], g_set0.g_commands[i + 2]};
            i += 3;
        }
        else if (command == 2.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            last_point = float2{g_set0.g_commands[i + 1], g_set0.g_commands[i + 2]};
            float2 v1 = last_point - v.shapecoord;
            coverage_x += line_test_x_axis(v0, v1, pixels_per_unit);
            coverage_y += line_test_y_axis(v0, v1, pixels_per_unit);
            i += 3;
        }
        else if (command == 3.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            float2 v1 = float2{g_set0.g_commands[i + 1], g_set0.g_commands[i + 2]} - v.shapecoord;
            last_point = float2{g_set0.g_commands[i + 3], g_set0.g_commands[i + 4]};
            float2 v2 = last_point - v.shapecoord;
            coverage_x += curve_test_x_axis(v0, v1, v2, pixels_per_unit);
            coverage_y += curve_test_y_axis(v0, v1, v2, pixels_per_unit);
            i += 5;
        }
        else if (command == 12.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            float2 v1 = float2{g_set0.g_commands[i + 1], g_set0.g_commands[i + 2]} - v.shapecoord;
            float2 v2 = float2{g_set0.g_commands[i + 3], g_set0.g_commands[i + 4]} - v.shapecoord;
            last_point = float2{g_set0.g_commands[i + 5], g_set0.g_commands[i + 6]};
            float2 v3 = last_point - v.shapecoord;
            coverage_x += cubic_curve_test_x_axis(v0, v1, v2, v3, pixels_per_unit);
            coverage_y += cubic_curve_test_y_axis(v0, v1, v2, v3, pixels_per_unit);
            i += 7;
        }
        else if (command >= 4.0f && command <= 7.0f)
        {
            float2 v0 = last_point - v.shapecoord;
            float radius = g_set0.g_commands[i + 1];
            float begin = g_set0.g_commands[i + 2];
            float command_arc_end = g_set0.g_commands[i + 3];
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
            float rx = g_set0.g_commands[i + 1];
            float ry = g_set0.g_commands[i + 2];
            float2 radius = float2{rx, ry};
            float begin = g_set0.g_commands[i + 3];
            float command_arc_end = g_set0.g_commands[i + 4];
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
    if (any(g_set0.g_cbuffer.clip_rect != float4{0.0f, 0.0f, 0.0f, 0.0f}))
    {
        coverage *= clip_rect_test(v.position_2d, g_set0.g_cbuffer.clip_rect);
    }

    PSOut o;
    float4 col = g_set0.g_tex.Sample(g_set0.g_sampler, v.texcoord);
    col *= v.color;
    col.w *= coverage;
    o.color = col;
    return o;
}
