/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file SDF.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include <Luna/GUI/SDF.hpp>
#include <cmath>

namespace Luna
{
    namespace GUI
    {
        namespace
        {
            struct ShapeNodeInfo
            {
                RectF bounds = RectF(0.0f, 0.0f, 0.0f, 0.0f);
                u32 next_float = 0;
                u32 num_instructions = 0;
                u32 max_stack_depth = 0;
            };

            struct ColorHeaderInfo
            {
                SDFColorInstruction instruction = SDFColorInstruction::solid;
                SDFClipMode clip_mode = SDFClipMode::none;
                f32 inner_distance = 0.0f;
                f32 outer_distance = 0.0f;
                u32 payload_float = 0;
            };

            struct ColorInstructionInfo
            {
                ColorHeaderInfo header;
                u32 next_float = 0;
                u32 num_stops = 0;
                Float2U shadow_offset = Float2U(0.0f);
                f32 shadow_softness = 0.0f;
                f32 shadow_spread = 0.0f;
            };

            bool decode_u32(f32 value, u32& result)
            {
                if(!std::isfinite(value) || value < 0.0f || value > 16777215.0f)
                {
                    return false;
                }
                f32 integral = std::floor(value);
                if(integral != value)
                {
                    return false;
                }
                result = (u32)integral;
                return true;
            }

            bool finite2(const Float2U& value)
            {
                return std::isfinite(value.x) && std::isfinite(value.y);
            }

            bool finite4(const Float4U& value)
            {
                return std::isfinite(value.x) && std::isfinite(value.y) &&
                    std::isfinite(value.z) && std::isfinite(value.w);
            }

            bool finite_rect(const RectF& rect)
            {
                return std::isfinite(rect.offset_x) && std::isfinite(rect.offset_y) &&
                    std::isfinite(rect.width) && std::isfinite(rect.height);
            }

            f32 clamp01(f32 value)
            {
                return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
            }

            f32 length2(f32 x, f32 y)
            {
                return std::sqrt(x * x + y * y);
            }

            RectF bounds_from_min_max(f32 min_x, f32 min_y, f32 max_x, f32 max_y)
            {
                return RectF(min_x, min_y, max_x - min_x, max_y - min_y);
            }

            RectF union_bounds(const RectF& lhs, const RectF& rhs)
            {
                f32 min_x = min(lhs.offset_x, rhs.offset_x);
                f32 min_y = min(lhs.offset_y, rhs.offset_y);
                f32 max_x = max(lhs.offset_x + lhs.width, rhs.offset_x + rhs.width);
                f32 max_y = max(lhs.offset_y + lhs.height, rhs.offset_y + rhs.height);
                return bounds_from_min_max(min_x, min_y, max_x, max_y);
            }

            RectF intersection_bounds(const RectF& lhs, const RectF& rhs)
            {
                f32 min_x = max(lhs.offset_x, rhs.offset_x);
                f32 min_y = max(lhs.offset_y, rhs.offset_y);
                f32 max_x = min(lhs.offset_x + lhs.width, rhs.offset_x + rhs.width);
                f32 max_y = min(lhs.offset_y + lhs.height, rhs.offset_y + rhs.height);
                return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
            }

            bool valid_clip(const SDFClipDesc& clip)
            {
                u32 mode = (u32)clip.mode;
                if(mode & ~SDF_COLOR_CLIP_MASK)
                {
                    return false;
                }
                if((mode & SDF_COLOR_INNER_CLIP_BIT) &&
                    (!std::isfinite(clip.inner_distance) || clip.inner_distance < 0.0f))
                {
                    return false;
                }
                if((mode & SDF_COLOR_OUTER_CLIP_BIT) &&
                    (!std::isfinite(clip.outer_distance) || clip.outer_distance < 0.0f))
                {
                    return false;
                }
                return true;
            }

            u32 append_color_header(Vector<f32>& floats, SDFColorInstruction instruction,
                const SDFClipDesc& clip)
            {
                u32 first_float = (u32)floats.size();
                u32 clip_bits = (u32)clip.mode & SDF_COLOR_CLIP_MASK;
                floats.push_back((f32)((u32)instruction | clip_bits));
                if(clip_bits & SDF_COLOR_INNER_CLIP_BIT)
                {
                    floats.push_back(clip.inner_distance);
                }
                if(clip_bits & SDF_COLOR_OUTER_CLIP_BIT)
                {
                    floats.push_back(clip.outer_distance);
                }
                return first_float;
            }

            void append_color(Vector<f32>& floats, const Float4U& color)
            {
                floats.push_back(color.x);
                floats.push_back(color.y);
                floats.push_back(color.z);
                floats.push_back(color.w);
            }

            Float4U read_color(Span<const f32> floats, u32 offset)
            {
                return Float4U(floats[offset], floats[offset + 1], floats[offset + 2],
                    floats[offset + 3]);
            }

            RV decode_color_header(Span<const f32> floats, u32 first_float, ColorHeaderInfo& output)
            {
                if(first_float >= floats.size())
                {
                    return set_error(BasicError::bad_data(), "SDF color instruction is truncated.");
                }
                u32 encoded_opcode = 0;
                if(!decode_u32(floats[first_float], encoded_opcode) ||
                    (encoded_opcode & ~(SDF_COLOR_OPCODE_MASK | SDF_COLOR_CLIP_MASK)))
                {
                    return set_error(BasicError::bad_data(), "SDF color opcode is invalid.");
                }
                u32 base_opcode = encoded_opcode & SDF_COLOR_OPCODE_MASK;
                if(base_opcode < (u32)SDFColorInstruction::solid ||
                    base_opcode > (u32)SDFColorInstruction::shadow)
                {
                    return set_error(BasicError::bad_data(), "Unknown SDF color instruction %u.", base_opcode);
                }
                output.instruction = (SDFColorInstruction)base_opcode;
                output.clip_mode = (SDFClipMode)(encoded_opcode & SDF_COLOR_CLIP_MASK);
                output.inner_distance = 0.0f;
                output.outer_distance = 0.0f;
                output.payload_float = first_float + 1;
                if(encoded_opcode & SDF_COLOR_INNER_CLIP_BIT)
                {
                    if(output.payload_float >= floats.size() ||
                        !std::isfinite(floats[output.payload_float]) || floats[output.payload_float] < 0.0f)
                    {
                        return set_error(BasicError::bad_data(), "SDF inner clip distance is invalid.");
                    }
                    output.inner_distance = floats[output.payload_float++];
                }
                if(encoded_opcode & SDF_COLOR_OUTER_CLIP_BIT)
                {
                    if(output.payload_float >= floats.size() ||
                        !std::isfinite(floats[output.payload_float]) || floats[output.payload_float] < 0.0f)
                    {
                        return set_error(BasicError::bad_data(), "SDF outer clip distance is invalid.");
                    }
                    output.outer_distance = floats[output.payload_float++];
                }
                return ok;
            }

            RV parse_color_instruction(Span<const f32> floats, u32 first_float,
                ColorInstructionInfo& output)
            {
                RV header_result = decode_color_header(floats, first_float, output.header);
                if(failed(header_result)) return header_result;
                u32 cursor = output.header.payload_float;
                switch(output.header.instruction)
                {
                case SDFColorInstruction::solid:
                    if(cursor + 4 > floats.size() || !finite4(read_color(floats, cursor)))
                    {
                        return set_error(BasicError::bad_data(), "Invalid SDF solid color instruction.");
                    }
                    output.next_float = cursor + 4;
                    break;
                case SDFColorInstruction::linear_gradient:
                case SDFColorInstruction::radial_gradient:
                {
                    if(cursor + 6 > floats.size())
                    {
                        return set_error(BasicError::bad_data(), "SDF gradient instruction is truncated.");
                    }
                    for(u32 i = 0; i < 4; ++i)
                    {
                        if(!std::isfinite(floats[cursor + i]))
                        {
                            return set_error(BasicError::bad_data(), "SDF gradient geometry is invalid.");
                        }
                    }
                    u32 spread = 0;
                    if(!decode_u32(floats[cursor + 4], spread) ||
                        spread > (u32)SDFGradientSpread::repeat ||
                        !decode_u32(floats[cursor + 5], output.num_stops))
                    {
                        return set_error(BasicError::bad_data(), "SDF gradient settings are invalid.");
                    }
                    cursor += 6;
                    if(output.header.instruction == SDFColorInstruction::radial_gradient &&
                        (floats[cursor - 4] <= 0.0f || floats[cursor - 3] <= 0.0f))
                    {
                        return set_error(BasicError::bad_data(),
                            "SDF radial gradient radii must be positive.");
                    }
                    break;
                }
                case SDFColorInstruction::conic_gradient:
                {
                    if(cursor + 5 > floats.size())
                    {
                        return set_error(BasicError::bad_data(),
                            "SDF conic gradient instruction is truncated.");
                    }
                    if(!std::isfinite(floats[cursor]) || !std::isfinite(floats[cursor + 1]) ||
                        !std::isfinite(floats[cursor + 2]))
                    {
                        return set_error(BasicError::bad_data(),
                            "SDF conic gradient geometry is invalid.");
                    }
                    u32 spread = 0;
                    if(!decode_u32(floats[cursor + 3], spread) ||
                        spread > (u32)SDFGradientSpread::repeat ||
                        !decode_u32(floats[cursor + 4], output.num_stops))
                    {
                        return set_error(BasicError::bad_data(), "SDF conic gradient settings are invalid.");
                    }
                    cursor += 5;
                    break;
                }
                case SDFColorInstruction::bilinear_gradient:
                    if(cursor + 20 > floats.size())
                    {
                        return set_error(BasicError::bad_data(),
                            "Invalid SDF bilinear gradient instruction size.");
                    }
                    for(u32 i = 0; i < 20; ++i)
                    {
                        if(!std::isfinite(floats[cursor + i]))
                        {
                            return set_error(BasicError::bad_data(),
                                "SDF bilinear gradient contains a non-finite parameter.");
                        }
                    }
                    output.next_float = cursor + 20;
                    break;
                case SDFColorInstruction::shadow:
                    if(cursor + 8 > floats.size() || !finite4(read_color(floats, cursor)))
                    {
                        return set_error(BasicError::bad_data(),
                            "Invalid SDF shadow instruction size or color.");
                    }
                    output.shadow_offset = Float2U(floats[cursor + 4], floats[cursor + 5]);
                    output.shadow_softness = floats[cursor + 6];
                    output.shadow_spread = floats[cursor + 7];
                    if(!finite2(output.shadow_offset) || !std::isfinite(output.shadow_softness) ||
                        output.shadow_softness < 0.0f || !std::isfinite(output.shadow_spread))
                    {
                        return set_error(BasicError::bad_data(), "SDF shadow parameters are invalid.");
                    }
                    output.next_float = cursor + 8;
                    break;
                default:
                    return set_error(BasicError::bad_data(), "Unknown SDF color instruction.");
                }

                if(output.header.instruction == SDFColorInstruction::linear_gradient ||
                    output.header.instruction == SDFColorInstruction::radial_gradient ||
                    output.header.instruction == SDFColorInstruction::conic_gradient)
                {
                    if(output.num_stops < 2 || output.num_stops > SDF_MAX_COLOR_STOPS ||
                        cursor + output.num_stops * 6 > floats.size())
                    {
                        return set_error(BasicError::bad_data(), "SDF gradient stop count is invalid.");
                    }
                    f32 former_position = -F32_INFINITY;
                    for(u32 i = 0; i < output.num_stops; ++i)
                    {
                        u32 stop = cursor + i * 6;
                        if(!std::isfinite(floats[stop]) || floats[stop] < former_position ||
                            !std::isfinite(floats[stop + 1]) || !finite4(read_color(floats, stop + 2)))
                        {
                            return set_error(BasicError::bad_data(),
                                "SDF gradient stops are invalid or unsorted.");
                        }
                        former_position = floats[stop];
                    }
                    output.next_float = cursor + output.num_stops * 6;
                }
                return ok;
            }

            RV parse_shape_node(Span<const f32> floats, u32 float_index, u32 recursion_depth,
                ShapeNodeInfo& output)
            {
                if(recursion_depth > SDF_MAX_SHAPE_STACK_DEPTH || float_index >= floats.size())
                {
                    return set_error(BasicError::bad_data(),
                        "SDF shape expression is truncated or too deeply nested.");
                }
                u32 instruction = 0;
                if(!decode_u32(floats[float_index], instruction))
                {
                    return set_error(BasicError::bad_data(), "SDF shape opcode is invalid.");
                }
                SDFShapeInstruction opcode = (SDFShapeInstruction)instruction;
                output.num_instructions = 1;
                output.max_stack_depth = 1;
                switch(opcode)
                {
                case SDFShapeInstruction::rectangle:
                    if(float_index + 5 > floats.size())
                    {
                        return set_error(BasicError::bad_data(), "SDF rectangle instruction is truncated.");
                    }
                    if(!std::isfinite(floats[float_index + 1]) || !std::isfinite(floats[float_index + 2]) ||
                        !std::isfinite(floats[float_index + 3]) || !std::isfinite(floats[float_index + 4]) ||
                        floats[float_index + 3] < floats[float_index + 1] ||
                        floats[float_index + 4] < floats[float_index + 2])
                    {
                        return set_error(BasicError::bad_data(), "SDF rectangle has invalid bounds.");
                    }
                    output.bounds = bounds_from_min_max(floats[float_index + 1], floats[float_index + 2],
                        floats[float_index + 3], floats[float_index + 4]);
                    output.next_float = float_index + 5;
                    return ok;
                case SDFShapeInstruction::rounded_rectangle:
                    if(float_index + 9 > floats.size())
                    {
                        return set_error(BasicError::bad_data(),
                            "SDF rounded rectangle instruction is truncated.");
                    }
                    for(u32 i = 1; i < 9; ++i)
                    {
                        if(!std::isfinite(floats[float_index + i]))
                        {
                            return set_error(BasicError::bad_data(),
                                "SDF rounded rectangle contains a non-finite parameter.");
                        }
                    }
                    if(floats[float_index + 3] < floats[float_index + 1] ||
                        floats[float_index + 4] < floats[float_index + 2] ||
                        floats[float_index + 5] < 0.0f || floats[float_index + 6] < 0.0f ||
                        floats[float_index + 7] < 0.0f || floats[float_index + 8] < 0.0f)
                    {
                        return set_error(BasicError::bad_data(),
                            "SDF rounded rectangle has invalid bounds or radii.");
                    }
                    output.bounds = bounds_from_min_max(floats[float_index + 1], floats[float_index + 2],
                        floats[float_index + 3], floats[float_index + 4]);
                    output.next_float = float_index + 9;
                    return ok;
                case SDFShapeInstruction::circle:
                    if(float_index + 4 > floats.size() || !std::isfinite(floats[float_index + 1]) ||
                        !std::isfinite(floats[float_index + 2]) || !std::isfinite(floats[float_index + 3]) ||
                        floats[float_index + 3] < 0.0f)
                    {
                        return set_error(BasicError::bad_data(), "Invalid SDF circle instruction.");
                    }
                    output.bounds = RectF(floats[float_index + 1] - floats[float_index + 3],
                        floats[float_index + 2] - floats[float_index + 3], floats[float_index + 3] * 2.0f,
                        floats[float_index + 3] * 2.0f);
                    output.next_float = float_index + 4;
                    return ok;
                case SDFShapeInstruction::ellipse:
                    if(float_index + 5 > floats.size() || !std::isfinite(floats[float_index + 1]) ||
                        !std::isfinite(floats[float_index + 2]) || !std::isfinite(floats[float_index + 3]) ||
                        !std::isfinite(floats[float_index + 4]) || floats[float_index + 3] <= 0.0f ||
                        floats[float_index + 4] <= 0.0f)
                    {
                        return set_error(BasicError::bad_data(), "Invalid SDF ellipse instruction.");
                    }
                    output.bounds = RectF(floats[float_index + 1] - floats[float_index + 3],
                        floats[float_index + 2] - floats[float_index + 4], floats[float_index + 3] * 2.0f,
                        floats[float_index + 4] * 2.0f);
                    output.next_float = float_index + 5;
                    return ok;
                case SDFShapeInstruction::capsule:
                    if(float_index + 6 > floats.size())
                    {
                        return set_error(BasicError::bad_data(), "SDF capsule instruction is truncated.");
                    }
                    for(u32 i = 1; i < 6; ++i)
                    {
                        if(!std::isfinite(floats[float_index + i]))
                        {
                            return set_error(BasicError::bad_data(),
                                "SDF capsule contains a non-finite parameter.");
                        }
                    }
                    if(floats[float_index + 5] < 0.0f)
                    {
                        return set_error(BasicError::bad_data(), "SDF capsule radius is negative.");
                    }
                    output.bounds = bounds_from_min_max(
                        min(floats[float_index + 1], floats[float_index + 3]) - floats[float_index + 5],
                        min(floats[float_index + 2], floats[float_index + 4]) - floats[float_index + 5],
                        max(floats[float_index + 1], floats[float_index + 3]) + floats[float_index + 5],
                        max(floats[float_index + 2], floats[float_index + 4]) + floats[float_index + 5]);
                    output.next_float = float_index + 6;
                    return ok;
                case SDFShapeInstruction::union_op:
                case SDFShapeInstruction::intersection_op:
                case SDFShapeInstruction::difference_op:
                case SDFShapeInstruction::xor_op:
                    break;
                default:
                    return set_error(BasicError::bad_data(), "Unknown SDF shape instruction %u.", instruction);
                }

                ShapeNodeInfo lhs;
                ShapeNodeInfo rhs;
                RV lhs_result = parse_shape_node(floats, float_index + 1, recursion_depth + 1, lhs);
                if(failed(lhs_result)) return lhs_result;
                RV rhs_result = parse_shape_node(floats, lhs.next_float, recursion_depth + 1, rhs);
                if(failed(rhs_result)) return rhs_result;
                output.next_float = rhs.next_float;
                output.num_instructions += lhs.num_instructions + rhs.num_instructions;
                output.max_stack_depth = max(rhs.max_stack_depth, 1u + lhs.max_stack_depth);
                switch(opcode)
                {
                case SDFShapeInstruction::union_op:
                case SDFShapeInstruction::xor_op:
                    output.bounds = union_bounds(lhs.bounds, rhs.bounds);
                    break;
                case SDFShapeInstruction::intersection_op:
                    output.bounds = intersection_bounds(lhs.bounds, rhs.bounds);
                    break;
                case SDFShapeInstruction::difference_op:
                    output.bounds = lhs.bounds;
                    break;
                default:
                    break;
                }
                return ok;
            }

            u32 shape_instruction_length(SDFShapeInstruction instruction)
            {
                switch(instruction)
                {
                case SDFShapeInstruction::rectangle: return 5;
                case SDFShapeInstruction::rounded_rectangle: return 9;
                case SDFShapeInstruction::circle: return 4;
                case SDFShapeInstruction::ellipse: return 5;
                case SDFShapeInstruction::capsule: return 6;
                case SDFShapeInstruction::union_op:
                case SDFShapeInstruction::intersection_op:
                case SDFShapeInstruction::difference_op:
                case SDFShapeInstruction::xor_op:
                    return 1;
                default:
                    return 0;
                }
            }

            bool valid_stops(Span<const SDFGradientStop> stops)
            {
                if(stops.size() < 2 || stops.size() > SDF_MAX_COLOR_STOPS)
                {
                    return false;
                }
                f32 former_position = stops[0].position;
                if(!std::isfinite(former_position) || !finite4(stops[0].color) ||
                    !std::isfinite(stops[0].midpoint))
                {
                    return false;
                }
                for(usize i = 1; i < stops.size(); ++i)
                {
                    if(!std::isfinite(stops[i].position) || stops[i].position < former_position ||
                        !finite4(stops[i].color) || !std::isfinite(stops[i].midpoint))
                    {
                        return false;
                    }
                    former_position = stops[i].position;
                }
                return true;
            }

            bool valid_spread(SDFGradientSpread spread)
            {
                return spread == SDFGradientSpread::pad || spread == SDFGradientSpread::repeat;
            }

            void append_stops(Vector<f32>& floats, Span<const SDFGradientStop> stops)
            {
                for(const SDFGradientStop& stop : stops)
                {
                    floats.push_back(stop.position);
                    floats.push_back(stop.midpoint);
                    append_color(floats, stop.color);
                }
            }

            f32 rectangle_distance(Span<const f32> floats, u32 offset, const Float2U& point)
            {
                f32 center_x = (floats[offset] + floats[offset + 2]) * 0.5f;
                f32 center_y = (floats[offset + 1] + floats[offset + 3]) * 0.5f;
                f32 half_x = (floats[offset + 2] - floats[offset]) * 0.5f;
                f32 half_y = (floats[offset + 3] - floats[offset + 1]) * 0.5f;
                f32 qx = std::fabs(point.x - center_x) - half_x;
                f32 qy = std::fabs(point.y - center_y) - half_y;
                return length2(max(qx, 0.0f), max(qy, 0.0f)) + min(max(qx, qy), 0.0f);
            }

            f32 rounded_rectangle_distance(Span<const f32> floats, u32 offset,
                const Float2U& point)
            {
                f32 center_x = (floats[offset] + floats[offset + 2]) * 0.5f;
                f32 center_y = (floats[offset + 1] + floats[offset + 3]) * 0.5f;
                f32 local_x = point.x - center_x;
                f32 local_y = point.y - center_y;
                f32 half_x = (floats[offset + 2] - floats[offset]) * 0.5f;
                f32 half_y = (floats[offset + 3] - floats[offset + 1]) * 0.5f;
                f32 radius = local_x < 0.0f ?
                    (local_y < 0.0f ? floats[offset + 4] : floats[offset + 7]) :
                    (local_y < 0.0f ? floats[offset + 5] : floats[offset + 6]);
                radius = min(radius, min(half_x, half_y));
                f32 qx = std::fabs(local_x) - half_x + radius;
                f32 qy = std::fabs(local_y) - half_y + radius;
                return length2(max(qx, 0.0f), max(qy, 0.0f)) + min(max(qx, qy), 0.0f) - radius;
            }

            f32 capsule_distance(Span<const f32> floats, u32 offset, const Float2U& point)
            {
                f32 pax = point.x - floats[offset];
                f32 pay = point.y - floats[offset + 1];
                f32 bax = floats[offset + 2] - floats[offset];
                f32 bay = floats[offset + 3] - floats[offset + 1];
                f32 denominator = bax * bax + bay * bay;
                f32 h = denominator > 0.0f ? clamp01((pax * bax + pay * bay) / denominator) : 0.0f;
                return length2(pax - bax * h, pay - bay * h) - floats[offset + 4];
            }

            Float4U lerp_color(const Float4U& lhs, const Float4U& rhs, f32 amount)
            {
                return Float4U(lhs.x + (rhs.x - lhs.x) * amount,
                    lhs.y + (rhs.y - lhs.y) * amount,
                    lhs.z + (rhs.z - lhs.z) * amount,
                    lhs.w + (rhs.w - lhs.w) * amount);
            }

            Float4U evaluate_stops(Span<const f32> floats, u32 stop_base, u32 num_stops,
                f32 coordinate, SDFGradientSpread spread)
            {
                f32 first_position = floats[stop_base];
                u32 last_base = stop_base + (num_stops - 1) * 6;
                f32 last_position = floats[last_base];
                if(spread == SDFGradientSpread::repeat && last_position > first_position)
                {
                    f32 period = last_position - first_position;
                    f32 period_coordinate = (coordinate - first_position) / period;
                    coordinate = first_position + (period_coordinate - std::floor(period_coordinate)) * period;
                }
                if(coordinate <= first_position)
                {
                    return read_color(floats, stop_base + 2);
                }
                if(coordinate >= last_position)
                {
                    return read_color(floats, last_base + 2);
                }
                for(u32 i = 0; i + 1 < num_stops; ++i)
                {
                    u32 lhs_base = stop_base + i * 6;
                    u32 rhs_base = lhs_base + 6;
                    f32 lhs_position = floats[lhs_base];
                    f32 rhs_position = floats[rhs_base];
                    if(coordinate <= rhs_position)
                    {
                        f32 width = rhs_position - lhs_position;
                        f32 amount = width > 0.0f ?
                            clamp01((coordinate - lhs_position) / width) : 1.0f;
                        f32 midpoint = floats[lhs_base + 1];
                        if(midpoint > 0.0f && midpoint < 1.0f && midpoint != 0.5f && amount > 0.0f)
                        {
                            amount = std::pow(amount, std::log(0.5f) / std::log(midpoint));
                        }
                        return lerp_color(read_color(floats, lhs_base + 2),
                            read_color(floats, rhs_base + 2), amount);
                    }
                }
                return read_color(floats, last_base + 2);
            }
        }

        void sdf_shape_add_operation(Vector<f32>& floats, SDFShapeInstruction operation)
        {
            floats.push_back((f32)(u32)operation);
        }

        void sdf_shape_add_rectangle(Vector<f32>& floats, const RectF& rect)
        {
            floats.push_back((f32)(u32)SDFShapeInstruction::rectangle);
            floats.push_back(rect.offset_x);
            floats.push_back(rect.offset_y);
            floats.push_back(rect.offset_x + rect.width);
            floats.push_back(rect.offset_y + rect.height);
        }

        void sdf_shape_add_rounded_rectangle(Vector<f32>& floats, const RectF& rect,
            const Float4U& corner_radii)
        {
            floats.push_back((f32)(u32)SDFShapeInstruction::rounded_rectangle);
            floats.push_back(rect.offset_x);
            floats.push_back(rect.offset_y);
            floats.push_back(rect.offset_x + rect.width);
            floats.push_back(rect.offset_y + rect.height);
            append_color(floats, corner_radii);
        }

        void sdf_shape_add_circle(Vector<f32>& floats, const Float2U& center, f32 radius)
        {
            floats.push_back((f32)(u32)SDFShapeInstruction::circle);
            floats.push_back(center.x);
            floats.push_back(center.y);
            floats.push_back(radius);
        }

        void sdf_shape_add_ellipse(Vector<f32>& floats, const Float2U& center, const Float2U& radii)
        {
            floats.push_back((f32)(u32)SDFShapeInstruction::ellipse);
            floats.push_back(center.x);
            floats.push_back(center.y);
            floats.push_back(radii.x);
            floats.push_back(radii.y);
        }

        void sdf_shape_add_capsule(Vector<f32>& floats, const Float2U& point0, const Float2U& point1,
            f32 radius)
        {
            floats.push_back((f32)(u32)SDFShapeInstruction::capsule);
            floats.push_back(point0.x);
            floats.push_back(point0.y);
            floats.push_back(point1.x);
            floats.push_back(point1.y);
            floats.push_back(radius);
        }

        SDFBufferRange sdf_color_add_solid(Vector<f32>& floats, const Float4U& color,
            const SDFClipDesc& clip)
        {
            if(!valid_clip(clip) || !finite4(color)) return SDFBufferRange();
            u32 first_float = append_color_header(floats, SDFColorInstruction::solid, clip);
            append_color(floats, color);
            return SDFBufferRange { first_float, (u32)floats.size() - first_float };
        }

        SDFBufferRange sdf_color_add_linear_gradient(Vector<f32>& floats, const Float2U& start,
            const Float2U& end, Span<const SDFGradientStop> stops, SDFGradientSpread spread,
            const SDFClipDesc& clip)
        {
            if(!valid_clip(clip) || !finite2(start) || !finite2(end) || !valid_stops(stops) ||
                !valid_spread(spread))
            {
                return SDFBufferRange();
            }
            u32 first_float = append_color_header(floats, SDFColorInstruction::linear_gradient, clip);
            floats.push_back(start.x);
            floats.push_back(start.y);
            floats.push_back(end.x);
            floats.push_back(end.y);
            floats.push_back((f32)(u32)spread);
            floats.push_back((f32)stops.size());
            append_stops(floats, stops);
            return SDFBufferRange { first_float, (u32)floats.size() - first_float };
        }

        SDFBufferRange sdf_color_add_radial_gradient(Vector<f32>& floats, const Float2U& center,
            const Float2U& radii, Span<const SDFGradientStop> stops, SDFGradientSpread spread,
            const SDFClipDesc& clip)
        {
            if(!valid_clip(clip) || !finite2(center) || !finite2(radii) || radii.x <= 0.0f ||
                radii.y <= 0.0f || !valid_stops(stops) || !valid_spread(spread))
            {
                return SDFBufferRange();
            }
            u32 first_float = append_color_header(floats, SDFColorInstruction::radial_gradient, clip);
            floats.push_back(center.x);
            floats.push_back(center.y);
            floats.push_back(radii.x);
            floats.push_back(radii.y);
            floats.push_back((f32)(u32)spread);
            floats.push_back((f32)stops.size());
            append_stops(floats, stops);
            return SDFBufferRange { first_float, (u32)floats.size() - first_float };
        }

        SDFBufferRange sdf_color_add_conic_gradient(Vector<f32>& floats, const Float2U& center,
            f32 start_angle, Span<const SDFGradientStop> stops, SDFGradientSpread spread,
            const SDFClipDesc& clip)
        {
            if(!valid_clip(clip) || !finite2(center) || !std::isfinite(start_angle) ||
                !valid_stops(stops) || !valid_spread(spread))
            {
                return SDFBufferRange();
            }
            u32 first_float = append_color_header(floats, SDFColorInstruction::conic_gradient, clip);
            floats.push_back(center.x);
            floats.push_back(center.y);
            floats.push_back(start_angle);
            floats.push_back((f32)(u32)spread);
            floats.push_back((f32)stops.size());
            append_stops(floats, stops);
            return SDFBufferRange { first_float, (u32)floats.size() - first_float };
        }

        SDFBufferRange sdf_color_add_bilinear_gradient(Vector<f32>& floats, const RectF& paint_rect,
            const Float4U& top_left, const Float4U& top_right, const Float4U& bottom_right,
            const Float4U& bottom_left, const SDFClipDesc& clip)
        {
            if(!valid_clip(clip) || !finite_rect(paint_rect) || !finite4(top_left) ||
                !finite4(top_right) || !finite4(bottom_right) || !finite4(bottom_left))
            {
                return SDFBufferRange();
            }
            u32 first_float = append_color_header(floats, SDFColorInstruction::bilinear_gradient, clip);
            floats.push_back(paint_rect.offset_x);
            floats.push_back(paint_rect.offset_y);
            floats.push_back(paint_rect.width);
            floats.push_back(paint_rect.height);
            append_color(floats, top_left);
            append_color(floats, top_right);
            append_color(floats, bottom_right);
            append_color(floats, bottom_left);
            return SDFBufferRange { first_float, (u32)floats.size() - first_float };
        }

        SDFBufferRange sdf_color_add_shadow(Vector<f32>& floats, const Float4U& color,
            const Float2U& offset, f32 softness, f32 spread, const SDFClipDesc& clip)
        {
            if(!valid_clip(clip) || !finite4(color) || !finite2(offset) || !std::isfinite(softness) ||
                softness < 0.0f || !std::isfinite(spread))
            {
                return SDFBufferRange();
            }
            u32 first_float = append_color_header(floats, SDFColorInstruction::shadow, clip);
            append_color(floats, color);
            floats.push_back(offset.x);
            floats.push_back(offset.y);
            floats.push_back(softness);
            floats.push_back(spread);
            return SDFBufferRange { first_float, (u32)floats.size() - first_float };
        }

        RV validate_sdf_shape_program(Span<const f32> floats, SDFShapeProgram* out_program)
        {
            if(floats.empty())
            {
                return set_error(BasicError::bad_data(), "SDF shape program is empty.");
            }
            if(floats.size() > SDF_MAX_SHAPE_FLOATS)
            {
                return set_error(BasicError::bad_data(), "SDF shape program exceeds the float limit.");
            }
            ShapeNodeInfo root;
            RV parse_result = parse_shape_node(floats, 0, 1, root);
            if(failed(parse_result)) return parse_result;
            if(root.next_float != floats.size())
            {
                return set_error(BasicError::bad_data(),
                    "SDF shape program contains trailing instructions or parameters.");
            }
            if(root.num_instructions > SDF_MAX_SHAPE_INSTRUCTIONS ||
                root.max_stack_depth > SDF_MAX_SHAPE_STACK_DEPTH)
            {
                return set_error(BasicError::bad_data(), "SDF shape program exceeds interpreter limits.");
            }
            if(out_program)
            {
                out_program->floats = SDFBufferRange { 0, (u32)floats.size() };
                out_program->bounds = root.bounds;
                out_program->num_instructions = root.num_instructions;
                out_program->max_stack_depth = root.max_stack_depth;
            }
            return ok;
        }

        RV validate_sdf_color_program(Span<const f32> floats, SDFColorProgram* out_program)
        {
            if(floats.empty())
            {
                return set_error(BasicError::bad_data(), "SDF color program is empty.");
            }
            if(floats.size() > SDF_MAX_COLOR_FLOATS)
            {
                return set_error(BasicError::bad_data(), "SDF color program exceeds the float limit.");
            }

            u32 cursor = 0;
            u32 num_instructions = 0;
            Float4U effect_outsets(0.0f);
            bool uses_shape_bounds = false;
            bool uses_raster_domain = false;
            bool may_paint_outside = false;
            bool has_unbounded_non_shadow = false;
            while(cursor < floats.size())
            {
                if(num_instructions >= SDF_MAX_COLOR_INSTRUCTIONS)
                {
                    return set_error(BasicError::bad_data(),
                        "SDF color program exceeds the instruction limit.");
                }
                ColorInstructionInfo instruction;
                RV parse_result = parse_color_instruction(floats, cursor, instruction);
                if(failed(parse_result)) return parse_result;
                if(instruction.next_float <= cursor)
                {
                    return set_error(BasicError::bad_data(), "SDF color instruction has invalid length.");
                }

                u32 clip_mode = (u32)instruction.header.clip_mode;
                if(instruction.header.instruction == SDFColorInstruction::shadow)
                {
                    f32 margin = max(instruction.shadow_spread, 0.0f) +
                        instruction.shadow_softness * 3.0f + 1.0f;
                    effect_outsets.x = max(effect_outsets.x,
                        max(1.0f, margin - instruction.shadow_offset.x));
                    effect_outsets.y = max(effect_outsets.y,
                        max(1.0f, margin - instruction.shadow_offset.y));
                    effect_outsets.z = max(effect_outsets.z,
                        max(1.0f, margin + instruction.shadow_offset.x));
                    effect_outsets.w = max(effect_outsets.w,
                        max(1.0f, margin + instruction.shadow_offset.y));
                    uses_shape_bounds = true;
                }
                else if(clip_mode & SDF_COLOR_OUTER_CLIP_BIT)
                {
                    f32 outset = instruction.header.outer_distance + 1.0f;
                    effect_outsets.x = max(effect_outsets.x, outset);
                    effect_outsets.y = max(effect_outsets.y, outset);
                    effect_outsets.z = max(effect_outsets.z, outset);
                    effect_outsets.w = max(effect_outsets.w, outset);
                    uses_shape_bounds = true;
                }
                else
                {
                    uses_raster_domain = true;
                    has_unbounded_non_shadow = true;
                }
                if(!(clip_mode & SDF_COLOR_OUTER_CLIP_BIT) ||
                    instruction.header.outer_distance > 0.0f)
                {
                    may_paint_outside = true;
                }

                cursor = instruction.next_float;
                ++num_instructions;
            }
            if(num_instructions > 1 && has_unbounded_non_shadow)
            {
                return set_error(BasicError::bad_data(),
                    "Multi-instruction SDF color programs require outer clipping on non-shadow paints.");
            }

            if(out_program)
            {
                out_program->floats = SDFBufferRange { 0, (u32)floats.size() };
                out_program->num_instructions = num_instructions;
                out_program->effect_outsets = effect_outsets;
                out_program->uses_shape_bounds = uses_shape_bounds;
                out_program->uses_raster_domain = uses_raster_domain;
                out_program->may_paint_outside = may_paint_outside;
            }
            return ok;
        }

        R<f32> evaluate_sdf_shape(Span<const f32> floats, const Float2U& point)
        {
            SDFShapeProgram program;
            RV validation = validate_sdf_shape_program(floats, &program);
            if(failed(validation)) return validation.errcode();

            u32 instruction_offsets[SDF_MAX_SHAPE_INSTRUCTIONS];
            u32 instruction_count = 0;
            u32 float_index = 0;
            while(float_index < floats.size())
            {
                u32 instruction = 0;
                if(!decode_u32(floats[float_index], instruction)) return BasicError::bad_data();
                u32 num_floats = shape_instruction_length((SDFShapeInstruction)instruction);
                if(!num_floats) return BasicError::bad_data();
                instruction_offsets[instruction_count++] = float_index;
                float_index += num_floats;
            }

            f32 stack[SDF_MAX_SHAPE_STACK_DEPTH];
            u32 stack_size = 0;
            for(i32 instruction_index = (i32)instruction_count - 1; instruction_index >= 0;
                --instruction_index)
            {
                u32 offset = instruction_offsets[instruction_index];
                SDFShapeInstruction instruction = (SDFShapeInstruction)(u32)floats[offset];
                f32 distance = 0.0f;
                switch(instruction)
                {
                case SDFShapeInstruction::rectangle:
                    distance = rectangle_distance(floats, offset + 1, point);
                    break;
                case SDFShapeInstruction::rounded_rectangle:
                    distance = rounded_rectangle_distance(floats, offset + 1, point);
                    break;
                case SDFShapeInstruction::circle:
                    distance = length2(point.x - floats[offset + 1], point.y - floats[offset + 2]) -
                        floats[offset + 3];
                    break;
                case SDFShapeInstruction::ellipse:
                {
                    f32 normalized = length2((point.x - floats[offset + 1]) / floats[offset + 3],
                        (point.y - floats[offset + 2]) / floats[offset + 4]);
                    distance = (normalized - 1.0f) * min(floats[offset + 3], floats[offset + 4]);
                    break;
                }
                case SDFShapeInstruction::capsule:
                    distance = capsule_distance(floats, offset + 1, point);
                    break;
                case SDFShapeInstruction::union_op:
                case SDFShapeInstruction::intersection_op:
                case SDFShapeInstruction::difference_op:
                case SDFShapeInstruction::xor_op:
                {
                    f32 lhs = stack[--stack_size];
                    f32 rhs = stack[--stack_size];
                    if(instruction == SDFShapeInstruction::union_op) distance = min(lhs, rhs);
                    else if(instruction == SDFShapeInstruction::intersection_op) distance = max(lhs, rhs);
                    else if(instruction == SDFShapeInstruction::difference_op) distance = max(lhs, -rhs);
                    else distance = max(min(lhs, rhs), -max(lhs, rhs));
                    break;
                }
                default:
                    return BasicError::bad_data();
                }
                stack[stack_size++] = distance;
            }
            return stack[0];
        }

        R<f32> evaluate_sdf_clip(Span<const f32> floats, f32 distance)
        {
            if(!std::isfinite(distance)) return BasicError::bad_data();
            SDFColorProgram program;
            RV validation = validate_sdf_color_program(floats, &program);
            if(failed(validation)) return validation.errcode();
            if(program.num_instructions != 1) return BasicError::bad_data();
            ColorInstructionInfo instruction;
            RV parse_result = parse_color_instruction(floats, 0, instruction);
            if(failed(parse_result)) return parse_result.errcode();
            u32 mode = (u32)instruction.header.clip_mode;
            if((mode & SDF_COLOR_INNER_CLIP_BIT) && distance < -instruction.header.inner_distance)
            {
                return 0.0f;
            }
            if((mode & SDF_COLOR_OUTER_CLIP_BIT) && distance > instruction.header.outer_distance)
            {
                return 0.0f;
            }
            return 1.0f;
        }

        R<Float4U> evaluate_sdf_color(Span<const f32> floats, const Float2U& point)
        {
            SDFColorProgram program;
            RV validation = validate_sdf_color_program(floats, &program);
            if(failed(validation)) return validation.errcode();
            if(program.num_instructions != 1) return BasicError::bad_data();
            ColorInstructionInfo instruction;
            RV parse_result = parse_color_instruction(floats, 0, instruction);
            if(failed(parse_result)) return parse_result.errcode();
            u32 cursor = instruction.header.payload_float;
            if(instruction.header.instruction == SDFColorInstruction::solid ||
                instruction.header.instruction == SDFColorInstruction::shadow)
            {
                return read_color(floats, cursor);
            }
            if(instruction.header.instruction == SDFColorInstruction::bilinear_gradient)
            {
                f32 x = floats[cursor + 2] != 0.0f ?
                    clamp01((point.x - floats[cursor]) / floats[cursor + 2]) : 0.0f;
                f32 y = floats[cursor + 3] != 0.0f ?
                    clamp01((point.y - floats[cursor + 1]) / floats[cursor + 3]) : 0.0f;
                return lerp_color(lerp_color(read_color(floats, cursor + 4),
                    read_color(floats, cursor + 8), x), lerp_color(read_color(floats, cursor + 16),
                    read_color(floats, cursor + 12), x), y);
            }

            f32 coordinate = 0.0f;
            u32 stop_base = 0;
            SDFGradientSpread spread = SDFGradientSpread::pad;
            if(instruction.header.instruction == SDFColorInstruction::linear_gradient)
            {
                f32 dx = floats[cursor + 2] - floats[cursor];
                f32 dy = floats[cursor + 3] - floats[cursor + 1];
                f32 denominator = dx * dx + dy * dy;
                coordinate = denominator > 0.0f ?
                    ((point.x - floats[cursor]) * dx + (point.y - floats[cursor + 1]) * dy) /
                    denominator : 0.0f;
                spread = (SDFGradientSpread)(u32)floats[cursor + 4];
                stop_base = cursor + 6;
            }
            else if(instruction.header.instruction == SDFColorInstruction::radial_gradient)
            {
                coordinate = length2((point.x - floats[cursor]) / floats[cursor + 2],
                    (point.y - floats[cursor + 1]) / floats[cursor + 3]);
                spread = (SDFGradientSpread)(u32)floats[cursor + 4];
                stop_base = cursor + 6;
            }
            else
            {
                coordinate = (std::atan2(point.y - floats[cursor + 1], point.x - floats[cursor]) -
                    floats[cursor + 2]) / TWO_PI;
                coordinate -= std::floor(coordinate);
                spread = (SDFGradientSpread)(u32)floats[cursor + 3];
                stop_base = cursor + 5;
            }
            return evaluate_stops(floats, stop_base, instruction.num_stops, coordinate, spread);
        }
    }
}
