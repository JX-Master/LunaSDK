#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct SDFFrameParams
{
    float4x4 surface_to_clip;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    SDFFrameParams frame;

    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const float* shape_floats;

    [[cppsl::structured_buffer, cppsl::binding(2)]]
    const float* color_floats;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

struct PSInput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(0)]] float2 surface_position;
    [[cppsl::location(1)]] float2 evaluation_origin;
    [[cppsl::location(2)]] float4 clip_rect;
    [[cppsl::location(3)]] float4 rounded_clip_rect;
    [[cppsl::location(4)]] float4 rounded_clip_radii;
    [[cppsl::location(5)]] uint4 program_data;
};

struct PSOutput
{
    [[cppsl::location(0)]] float4 color;
};

struct ShapeResult
{
    float distance;
    bool valid;
};

struct ColorResult
{
    float4 color;
    uint next_float;
    bool valid;
};

ShapeResult make_shape_result(float distance, bool valid)
{
    ShapeResult result;
    result.distance = distance;
    result.valid = valid;
    return result;
}

ColorResult make_color_result(float4 color, uint next_float, bool valid)
{
    ColorResult result;
    result.color = color;
    result.next_float = next_float;
    result.valid = valid;
    return result;
}

float4 read_color(uint offset)
{
    return float4{g_set0.color_floats[offset], g_set0.color_floats[offset + 1],
        g_set0.color_floats[offset + 2], g_set0.color_floats[offset + 3]};
}

float rectangle_distance(float2 point, uint offset)
{
    float4 rect = float4{g_set0.shape_floats[offset], g_set0.shape_floats[offset + 1],
        g_set0.shape_floats[offset + 2], g_set0.shape_floats[offset + 3]};
    float2 half_size = (rect.zw - rect.xy) * 0.5f;
    float2 center = rect.xy + half_size;
    float2 q = abs(point - center) - half_size;
    return sqrt(dot(max(q, float2{0.0f, 0.0f}), max(q, float2{0.0f, 0.0f}))) +
        min(max(q.x, q.y), 0.0f);
}

float rounded_rectangle_distance(float2 point, uint offset)
{
    float4 rect = float4{g_set0.shape_floats[offset], g_set0.shape_floats[offset + 1],
        g_set0.shape_floats[offset + 2], g_set0.shape_floats[offset + 3]};
    float4 radii = float4{g_set0.shape_floats[offset + 4], g_set0.shape_floats[offset + 5],
        g_set0.shape_floats[offset + 6], g_set0.shape_floats[offset + 7]};
    float2 half_size = (rect.zw - rect.xy) * 0.5f;
    float2 center = rect.xy + half_size;
    float2 local = point - center;
    float radius;
    if(local.x < 0.0f)
    {
        radius = local.y < 0.0f ? radii.x : radii.w;
    }
    else
    {
        radius = local.y < 0.0f ? radii.y : radii.z;
    }
    radius = min(radius, min(half_size.x, half_size.y));
    float2 q = abs(local) - half_size + radius;
    return sqrt(dot(max(q, float2{0.0f, 0.0f}), max(q, float2{0.0f, 0.0f}))) +
        min(max(q.x, q.y), 0.0f) - radius;
}

float capsule_distance(float2 point, uint offset)
{
    float2 point0 = float2{g_set0.shape_floats[offset], g_set0.shape_floats[offset + 1]};
    float2 point1 = float2{g_set0.shape_floats[offset + 2], g_set0.shape_floats[offset + 3]};
    float radius = g_set0.shape_floats[offset + 4];
    float2 point_from_start = point - point0;
    float2 segment = point1 - point0;
    float denominator = dot(segment, segment);
    float amount = denominator > 0.0f ? saturate(dot(point_from_start, segment) / denominator) : 0.0f;
    float2 difference = point_from_start - segment * amount;
    return sqrt(dot(difference, difference)) - radius;
}

uint shape_instruction_length(uint opcode)
{
    if(opcode == 1) return 5;
    if(opcode == 2) return 9;
    if(opcode == 3) return 4;
    if(opcode == 4) return 5;
    if(opcode == 5) return 6;
    if(opcode >= 16 && opcode <= 19) return 1;
    return 0;
}

ShapeResult evaluate_primitive(float2 point, uint offset, uint opcode)
{
    if(opcode == 1)
    {
        return make_shape_result(rectangle_distance(point, offset + 1), true);
    }
    if(opcode == 2)
    {
        return make_shape_result(rounded_rectangle_distance(point, offset + 1), true);
    }
    if(opcode == 3)
    {
        float2 center = float2{g_set0.shape_floats[offset + 1],
            g_set0.shape_floats[offset + 2]};
        float radius = g_set0.shape_floats[offset + 3];
        float2 difference = point - center;
        return make_shape_result(sqrt(dot(difference, difference)) - radius, true);
    }
    if(opcode == 4)
    {
        float2 center = float2{g_set0.shape_floats[offset + 1],
            g_set0.shape_floats[offset + 2]};
        float2 radii = float2{g_set0.shape_floats[offset + 3],
            g_set0.shape_floats[offset + 4]};
        float2 normalized = (point - center) / radii;
        return make_shape_result((sqrt(dot(normalized, normalized)) - 1.0f) *
            min(radii.x, radii.y), true);
    }
    if(opcode == 5)
    {
        return make_shape_result(capsule_distance(point, offset + 1), true);
    }
    return make_shape_result(0.0f, false);
}

ShapeResult evaluate_shape(float2 point, uint first_float)
{
    uint first_opcode = (uint)g_set0.shape_floats[first_float];
    if(first_opcode >= 1 && first_opcode <= 5)
    {
        return evaluate_primitive(point, first_float, first_opcode);
    }

    uint offsets[64];
    uint instruction_count = 0;
    uint pending_expressions = 1;
    uint cursor = first_float;
    while(pending_expressions != 0 && instruction_count < 64)
    {
        uint opcode = (uint)g_set0.shape_floats[cursor];
        uint instruction_length = shape_instruction_length(opcode);
        if(instruction_length == 0)
        {
            return make_shape_result(0.0f, false);
        }
        offsets[instruction_count] = cursor;
        instruction_count += 1;
        cursor += instruction_length;
        if(opcode >= 1 && opcode <= 5)
        {
            pending_expressions -= 1;
        }
        else
        {
            pending_expressions += 1;
        }
    }
    if(pending_expressions != 0 || instruction_count == 0)
    {
        return make_shape_result(0.0f, false);
    }

    float distances[16];
    uint stack_size = 0;
    int instruction_index = (int)instruction_count - 1;
    while(instruction_index >= 0)
    {
        uint offset = offsets[instruction_index];
        uint opcode = (uint)g_set0.shape_floats[offset];
        float distance_value = 0.0f;
        if(opcode >= 1 && opcode <= 5)
        {
            ShapeResult primitive = evaluate_primitive(point, offset, opcode);
            if(!primitive.valid)
            {
                return make_shape_result(0.0f, false);
            }
            distance_value = primitive.distance;
        }
        else
        {
            if(stack_size < 2)
            {
                return make_shape_result(0.0f, false);
            }
            float lhs = distances[stack_size - 1];
            float rhs = distances[stack_size - 2];
            stack_size -= 2;
            if(opcode == 16) distance_value = min(lhs, rhs);
            else if(opcode == 17) distance_value = max(lhs, rhs);
            else if(opcode == 18) distance_value = max(lhs, -rhs);
            else if(opcode == 19) distance_value = max(min(lhs, rhs), -max(lhs, rhs));
            else return make_shape_result(0.0f, false);
        }
        if(stack_size >= 16)
        {
            return make_shape_result(0.0f, false);
        }
        distances[stack_size] = distance_value;
        stack_size += 1;
        instruction_index -= 1;
    }
    if(stack_size != 1)
    {
        return make_shape_result(0.0f, false);
    }
    return make_shape_result(distances[0], true);
}

float4 evaluate_stops(uint stop_base, uint num_stops, float coordinate, uint spread)
{
    float first_position = g_set0.color_floats[stop_base];
    uint last_base = stop_base + (num_stops - 1) * 6;
    float last_position = g_set0.color_floats[last_base];
    if(spread == 1 && last_position > first_position)
    {
        float period = last_position - first_position;
        float period_coordinate = (coordinate - first_position) / period;
        coordinate = first_position + (period_coordinate - floor(period_coordinate)) * period;
    }
    if(coordinate <= first_position)
    {
        return read_color(stop_base + 2);
    }
    if(coordinate >= last_position)
    {
        return read_color(last_base + 2);
    }
    uint i = 0;
    while(i + 1 < num_stops && i < 16)
    {
        uint lhs_base = stop_base + i * 6;
        uint rhs_base = lhs_base + 6;
        float lhs_position = g_set0.color_floats[lhs_base];
        float rhs_position = g_set0.color_floats[rhs_base];
        if(coordinate <= rhs_position)
        {
            float width = rhs_position - lhs_position;
            float amount = width > 0.0f ? saturate((coordinate - lhs_position) / width) : 1.0f;
            float midpoint = g_set0.color_floats[lhs_base + 1];
            if(midpoint > 0.0f && midpoint < 1.0f && abs(midpoint - 0.5f) > 0.0001f && amount > 0.0f)
            {
                amount = pow(amount, -1.0f / log2(midpoint));
            }
            return lerp(read_color(lhs_base + 2), read_color(rhs_base + 2), amount);
        }
        i += 1;
    }
    return read_color(last_base + 2);
}

float4 evaluate_paint(float2 point, uint opcode, uint payload_float)
{
    if(opcode == 1)
    {
        return read_color(payload_float);
    }
    if(opcode == 5)
    {
        float4 rect = float4{g_set0.color_floats[payload_float],
            g_set0.color_floats[payload_float + 1], g_set0.color_floats[payload_float + 2],
            g_set0.color_floats[payload_float + 3]};
        float x = rect.z != 0.0f ? saturate((point.x - rect.x) / rect.z) : 0.0f;
        float y = rect.w != 0.0f ? saturate((point.y - rect.y) / rect.w) : 0.0f;
        float4 top = lerp(read_color(payload_float + 4), read_color(payload_float + 8), x);
        float4 bottom = lerp(read_color(payload_float + 16), read_color(payload_float + 12), x);
        return lerp(top, bottom, y);
    }

    float coordinate = 0.0f;
    uint spread = 0;
    uint num_stops = 0;
    uint stop_base = 0;
    if(opcode == 2)
    {
        float2 start = float2{g_set0.color_floats[payload_float],
            g_set0.color_floats[payload_float + 1]};
        float2 end = float2{g_set0.color_floats[payload_float + 2],
            g_set0.color_floats[payload_float + 3]};
        float2 direction = end - start;
        float denominator = dot(direction, direction);
        coordinate = denominator > 0.0f ? dot(point - start, direction) / denominator : 0.0f;
        spread = (uint)g_set0.color_floats[payload_float + 4];
        num_stops = (uint)g_set0.color_floats[payload_float + 5];
        stop_base = payload_float + 6;
    }
    else if(opcode == 3)
    {
        float2 center = float2{g_set0.color_floats[payload_float],
            g_set0.color_floats[payload_float + 1]};
        float2 radii = float2{g_set0.color_floats[payload_float + 2],
            g_set0.color_floats[payload_float + 3]};
        float2 normalized = (point - center) / radii;
        coordinate = sqrt(dot(normalized, normalized));
        spread = (uint)g_set0.color_floats[payload_float + 4];
        num_stops = (uint)g_set0.color_floats[payload_float + 5];
        stop_base = payload_float + 6;
    }
    else
    {
        float2 center = float2{g_set0.color_floats[payload_float],
            g_set0.color_floats[payload_float + 1]};
        float start_angle = g_set0.color_floats[payload_float + 2];
        coordinate = (atan2(point.y - center.y, point.x - center.x) - start_angle) / 6.283185307f;
        coordinate -= floor(coordinate);
        spread = (uint)g_set0.color_floats[payload_float + 3];
        num_stops = (uint)g_set0.color_floats[payload_float + 4];
        stop_base = payload_float + 5;
    }
    return evaluate_stops(stop_base, num_stops, coordinate, spread);
}

float normal_distribution_cdf(float value)
{
    float absolute_value = abs(value);
    float t = 1.0f / (1.0f + 0.2316419f * absolute_value);
    float density = 0.3989422804f * exp2(-0.72134752f * absolute_value * absolute_value);
    float tail = density * t * (0.319381530f + t * (-0.356563782f + t *
        (1.781477937f + t * (-1.821255978f + t * 1.330274429f))));
    return value >= 0.0f ? 1.0f - tail : tail;
}

float distance_coverage(float distance, float softness)
{
    float width = max(fwidth(distance), 0.0001f);
    if(softness > 0.0001f)
    {
        float filtered_softness = sqrt(softness * softness + width * width / 12.0f);
        return normal_distribution_cdf(-distance / filtered_softness);
    }
    return saturate(0.5f - distance / width);
}

float4 premultiply(float4 color, float coverage)
{
    color.w *= coverage;
    color.x *= color.w;
    color.y *= color.w;
    color.z *= color.w;
    return color;
}

PSOutput transparent_output()
{
    PSOutput result;
    result.color = float4{0.0f, 0.0f, 0.0f, 0.0f};
    return result;
}

ColorResult evaluate_color_instruction(float2 local_point, ShapeResult shape, uint shape_first,
    uint color_float, uint color_end, float raster_clip_coverage)
{
    if(color_float >= color_end)
    {
        return make_color_result(float4{0.0f, 0.0f, 0.0f, 0.0f}, color_float, false);
    }
    uint encoded_opcode = (uint)g_set0.color_floats[color_float];
    uint opcode = encoded_opcode & 0xFF;
    bool has_inner_clip = (encoded_opcode & 0x100) != 0;
    bool has_outer_clip = (encoded_opcode & 0x200) != 0;
    uint payload_float = color_float + 1;
    float clip_distance = -1000000.0f;
    if(has_inner_clip)
    {
        float inner_distance = g_set0.color_floats[payload_float];
        payload_float += 1;
        clip_distance = max(clip_distance, -shape.distance - inner_distance);
    }
    if(has_outer_clip)
    {
        float outer_distance = g_set0.color_floats[payload_float];
        payload_float += 1;
        clip_distance = max(clip_distance, shape.distance - outer_distance);
    }

    uint next_float = payload_float;
    if(opcode == 1)
    {
        next_float += 4;
    }
    else if(opcode == 2 || opcode == 3)
    {
        next_float += 6 + (uint)g_set0.color_floats[payload_float + 5] * 6;
    }
    else if(opcode == 4)
    {
        next_float += 5 + (uint)g_set0.color_floats[payload_float + 4] * 6;
    }
    else if(opcode == 5)
    {
        next_float += 20;
    }
    else if(opcode == 6)
    {
        next_float += 8;
    }
    else
    {
        return make_color_result(float4{0.0f, 0.0f, 0.0f, 0.0f}, color_float, false);
    }
    if(next_float > color_end)
    {
        return make_color_result(float4{0.0f, 0.0f, 0.0f, 0.0f}, color_float, false);
    }

    float clip_coverage = raster_clip_coverage * ((has_inner_clip || has_outer_clip) ?
        distance_coverage(clip_distance, 0.0f) : 1.0f);
    if(clip_coverage <= 0.0f)
    {
        return make_color_result(float4{0.0f, 0.0f, 0.0f, 0.0f}, next_float, true);
    }

    float4 output_color;
    if(opcode == 6)
    {
        float4 shadow_color = read_color(payload_float);
        float2 shadow_offset = float2{g_set0.color_floats[payload_float + 4],
            g_set0.color_floats[payload_float + 5]};
        float softness = g_set0.color_floats[payload_float + 6];
        float spread = g_set0.color_floats[payload_float + 7];
        ShapeResult shifted_shape = evaluate_shape(local_point - shadow_offset, shape_first);
        if(!shifted_shape.valid)
        {
            return make_color_result(float4{0.0f, 0.0f, 0.0f, 0.0f}, color_float, false);
        }
        float blurred_coverage = distance_coverage(shifted_shape.distance - spread, softness);
        float shadow_field;
        if(has_inner_clip && !has_outer_clip)
        {
            shadow_field = blurred_coverage;
        }
        else if(has_outer_clip && !has_inner_clip)
        {
            shadow_field = 1.0f - blurred_coverage;
        }
        else
        {
            float source_coverage = distance_coverage(shape.distance, 0.0f);
            shadow_field = blurred_coverage * (1.0f - source_coverage) +
                (1.0f - blurred_coverage) * source_coverage;
        }
        output_color = premultiply(shadow_color, shadow_field * clip_coverage);
    }
    else
    {
        output_color = premultiply(evaluate_paint(local_point, opcode, payload_float), clip_coverage);
    }
    return make_color_result(output_color, next_float, true);
}

float rounded_clip_rect_distance(float2 point, float4 clip_rect, float4 corner_radii)
{
    float2 half_size = clip_rect.zw * 0.5f;
    float2 center = clip_rect.xy + half_size;
    float2 local = point - center;
    float radius;
    if(local.x < 0.0f)
    {
        radius = local.y < 0.0f ? corner_radii.x : corner_radii.w;
    }
    else
    {
        radius = local.y < 0.0f ? corner_radii.y : corner_radii.z;
    }
    radius = min(max(radius, 0.0f), min(half_size.x, half_size.y));
    float2 q = abs(local) - half_size + radius;
    return sqrt(dot(max(q, float2{0.0f, 0.0f}), max(q, float2{0.0f, 0.0f}))) +
        min(max(q.x, q.y), 0.0f) - radius;
}

[[cppsl::fragment]]
PSOutput ps_main(PSInput pixel)
{
    float raster_clip_coverage = 1.0f;
    if((pixel.program_data.z & 1u) != 0)
    {
        float2 clip_max = pixel.clip_rect.xy + pixel.clip_rect.zw;
        bool inside_clip = pixel.surface_position.x >= pixel.clip_rect.x &&
            pixel.surface_position.y >= pixel.clip_rect.y && pixel.surface_position.x < clip_max.x &&
            pixel.surface_position.y < clip_max.y;
        if(!inside_clip)
        {
            return transparent_output();
        }
    }
    if((pixel.program_data.z & 2u) != 0)
    {
        float rounded_clip_distance = rounded_clip_rect_distance(pixel.surface_position,
            pixel.rounded_clip_rect, pixel.rounded_clip_radii);
        raster_clip_coverage = distance_coverage(rounded_clip_distance, 0.0f);
        if(raster_clip_coverage <= 0.0f)
        {
            return transparent_output();
        }
    }

    float2 local_point = pixel.surface_position - pixel.evaluation_origin;
    ShapeResult shape = evaluate_shape(local_point, pixel.program_data.x);
    if(!shape.valid)
    {
        return transparent_output();
    }

    uint num_color_floats = (pixel.program_data.z >> 2) & 0x7FF;
    if(num_color_floats == 0)
    {
        return transparent_output();
    }
    uint color_float = pixel.program_data.y;
    uint color_end = color_float + num_color_floats;
    ColorResult first_color = evaluate_color_instruction(local_point, shape, pixel.program_data.x,
        color_float, color_end, raster_clip_coverage);
    if(!first_color.valid)
    {
        return transparent_output();
    }
    if(first_color.next_float == color_end)
    {
        PSOutput result;
        result.color = first_color.color;
        return result;
    }
    if(first_color.next_float <= color_float || first_color.next_float > color_end)
    {
        return transparent_output();
    }

    float4 output_color = first_color.color;
    color_float = first_color.next_float;
    uint instruction_index = 1;
    while(color_float < color_end && instruction_index < 8)
    {
        ColorResult color = evaluate_color_instruction(local_point, shape, pixel.program_data.x,
            color_float, color_end, raster_clip_coverage);
        if(!color.valid || color.next_float <= color_float || color.next_float > color_end)
        {
            return transparent_output();
        }
        output_color = color.color + output_color * (1.0f - color.color.w);
        color_float = color.next_float;
        instruction_index += 1;
    }
    if(color_float != color_end)
    {
        return transparent_output();
    }
    PSOutput result;
    result.color = output_color;
    return result;
}
