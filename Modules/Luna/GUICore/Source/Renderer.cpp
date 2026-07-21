/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Renderer.cpp
* @author JXMaster
* @date 2026/7/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUICORE_API LUNA_EXPORT
#include "RendererImpl.hpp"
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <SDFVS.hpp>
#include <SDFPS.hpp>

namespace Luna
{
    namespace GUICore
    {
        namespace
        {
            static_assert(sizeof(SDFInstance) == sizeof(f32) * 10,
                "SDFInstance must match the instance input layout used by SDFVS.");
            static_assert(sizeof(SDFState) == sizeof(f32) * 12,
                "SDFState must match the structured-buffer layout used by SDFVS.");

            constexpr u32 SDF_INSTANCE_COLOR_FLOAT_COUNT_SHIFT = 2;
            static_assert(SDF_MAX_COLOR_FLOATS <= 0x7FF,
                "SDF color float count must fit the instance encoding.");

            struct SDFFrameBuffer
            {
                Float4U screen_params;
            };

            bool color_visible(const Float4U& color)
            {
                return color.w > 0.0f;
            }

            bool rect_visible(const RectF& rect)
            {
                return rect.width > 0.0f && rect.height > 0.0f;
            }

            RectF intersect_rect(const RectF& a, const RectF& b)
            {
                f32 min_x = max(a.offset_x, b.offset_x);
                f32 min_y = max(a.offset_y, b.offset_y);
                f32 max_x = min(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = min(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max(max_x - min_x, 0.0f), max(max_y - min_y, 0.0f));
            }

            RectF union_rect(const RectF& a, const RectF& b)
            {
                if(!rect_visible(a)) return b;
                if(!rect_visible(b)) return a;
                f32 min_x = min(a.offset_x, b.offset_x);
                f32 min_y = min(a.offset_y, b.offset_y);
                f32 max_x = max(a.offset_x + a.width, b.offset_x + b.width);
                f32 max_y = max(a.offset_y + a.height, b.offset_y + b.height);
                return RectF(min_x, min_y, max_x - min_x, max_y - min_y);
            }

            RectF expand_rect(const RectF& rect, f32 amount)
            {
                return RectF(rect.offset_x - amount, rect.offset_y - amount,
                    rect.width + amount * 2.0f, rect.height + amount * 2.0f);
            }

            RectF expand_rect(const RectF& rect, const Float4U& outsets)
            {
                return RectF(rect.offset_x - outsets.x, rect.offset_y - outsets.y,
                    rect.width + outsets.x + outsets.z, rect.height + outsets.y + outsets.w);
            }

            bool has_clip(const RectF& clip_rect)
            {
                return clip_rect.width > 0.0f || clip_rect.height > 0.0f;
            }

            struct ClipState
            {
                RectF rect;
                RectF rounded_rect;
                Float4U rounded_radii = Float4U(0.0f);
            };

            bool has_rounded_clip(const ClipState& clip)
            {
                return has_clip(clip.rounded_rect);
            }

            RectF merge_clip_rect(const RectF& a, const RectF& b)
            {
                if(has_clip(a) && has_clip(b))
                {
                    return intersect_rect(a, b);
                }
                return has_clip(a) ? a : b;
            }

            RectF to_screen_rect(Span<const Layer> layers, u32 layer_index, const RectF& rect)
            {
                if(layer_index >= layers.size())
                {
                    return rect;
                }
                const Float2U& layer_position = layers[layer_index].screen_position;
                return RectF(layer_position.x + rect.offset_x, layer_position.y + rect.offset_y,
                    rect.width, rect.height);
            }

            RectF to_vg_rect(const FrameDesc& frame_desc, const RectF& screen_rect)
            {
                return RectF(screen_rect.offset_x,
                    frame_desc.screen_size.y - screen_rect.offset_y - screen_rect.height,
                    screen_rect.width, screen_rect.height);
            }

            RectF resolve_draw_rect(const DrawCommand& command, Span<const Element> elements)
            {
                if(command.rect_reference != DrawCommandRectReference::element ||
                    command.element == INVALID_ELEMENT || command.element >= elements.size())
                {
                    return command.rect;
                }
                const RectF& element_rect = elements[command.element].layout_result.rect;
                RectF ret;
                ret.offset_x = element_rect.offset_x + command.rect.offset_x +
                    element_rect.width * command.rect_layout_scale.x;
                ret.offset_y = element_rect.offset_y + command.rect.offset_y +
                    element_rect.height * command.rect_layout_scale.y;
                f32 scaled_width = command.rect.width + element_rect.width * command.rect_layout_scale.z;
                f32 scaled_height = command.rect.height + element_rect.height * command.rect_layout_scale.w;
                ret.width = scaled_width > 0.0f ? scaled_width :
                    (command.rect.width < 0.0f ? max(element_rect.width + scaled_width, 1.0f) :
                        max(element_rect.width - command.rect.offset_x, 1.0f));
                ret.height = scaled_height > 0.0f ? scaled_height :
                    (command.rect.height < 0.0f ? max(element_rect.height + scaled_height, 1.0f) :
                        max(element_rect.height - command.rect.offset_y, 1.0f));
                return ret;
            }

            Float2U resolve_draw_point0(const DrawCommand& command, Span<const Element> elements)
            {
                if(command.rect_reference != DrawCommandRectReference::element ||
                    command.element == INVALID_ELEMENT || command.element >= elements.size())
                {
                    return Float2U(command.rect.offset_x, command.rect.offset_y);
                }
                const RectF& element_rect = elements[command.element].layout_result.rect;
                return Float2U(
                    element_rect.offset_x + command.rect.offset_x + element_rect.width * command.rect_layout_scale.x,
                    element_rect.offset_y + command.rect.offset_y + element_rect.height * command.rect_layout_scale.y);
            }

            Float2U resolve_draw_point1(const DrawCommand& command, Span<const Element> elements)
            {
                if(command.rect_reference != DrawCommandRectReference::element ||
                    command.element == INVALID_ELEMENT || command.element >= elements.size())
                {
                    return command.point1;
                }
                const RectF& element_rect = elements[command.element].layout_result.rect;
                return Float2U(
                    element_rect.offset_x + command.point1.x + element_rect.width * command.rect_layout_scale.z,
                    element_rect.offset_y + command.point1.y + element_rect.height * command.rect_layout_scale.w);
            }

            u32 append_program_floats(Vector<f32>& destination, Span<const f32> floats)
            {
                u32 page_offset = (u32)(destination.size() % SDF_PROGRAM_PAGE_FLOATS);
                if(page_offset && page_offset + floats.size() > SDF_PROGRAM_PAGE_FLOATS)
                {
                    destination.resize(destination.size() + SDF_PROGRAM_PAGE_FLOATS - page_offset, 0.0f);
                }
                u32 first_float = (u32)destination.size();
                destination.insert(destination.end(), floats.begin(), floats.end());
                return first_float;
            }

            bool append_shape_program(Vector<f32>& destination, Span<const f32> floats,
                SDFShapeProgram& program)
            {
                RV validation = validate_sdf_shape_program(floats, &program);
                if(failed(validation)) return false;
                program.floats.first_float = append_program_floats(destination, floats);
                return true;
            }

            bool append_color_program(Vector<f32>& destination, Span<const f32> floats,
                SDFColorProgram& program)
            {
                RV validation = validate_sdf_color_program(floats, &program);
                if(failed(validation)) return false;
                program.floats.first_float = append_program_floats(destination, floats);
                return true;
            }

            template <typename _Program>
            bool resolve_context_program(Span<const f32> all_floats, const SDFBufferRange& range,
                _Program& output, RV(*validator)(Span<const f32>, _Program*))
            {
                if(!range.num_floats || range.first_float > all_floats.size() ||
                    range.num_floats > all_floats.size() - range.first_float ||
                    range.first_float / SDF_PROGRAM_PAGE_FLOATS !=
                    (range.first_float + range.num_floats - 1) / SDF_PROGRAM_PAGE_FLOATS)
                {
                    return false;
                }
                Span<const f32> program_floats(all_floats.data() + range.first_float, range.num_floats);
                RV validation = validator(program_floats, &output);
                if(failed(validation)) return false;
                output.floats = range;
                return true;
            }

            f64 perf_elapsed_ms(u64 begin, u64 end)
            {
                return (f64)(end - begin) * 1000.0 / get_ticks_per_second();
            }
        }

        RV Renderer::init(RHI::IDevice* device)
        {
            using namespace RHI;
            lutry
            {
                m_device = device ? device : get_main_device();
                lucheck(m_device);
                m_draw_list = VG::new_shape_draw_list(m_device);
                m_shape_renderer = VG::new_fill_shape_renderer();
                m_font_atlas = VG::new_font_atlas();

                DescriptorSetLayoutBinding bindings[] = {
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all),
                    DescriptorSetLayoutBinding::read_buffer_view(1, 1, ShaderVisibilityFlag::pixel),
                    DescriptorSetLayoutBinding::read_buffer_view(2, 1, ShaderVisibilityFlag::pixel),
                    DescriptorSetLayoutBinding::read_buffer_view(3, 1, ShaderVisibilityFlag::vertex)
                };
                luset(m_sdf_descriptor_set_layout,
                    m_device->new_descriptor_set_layout(DescriptorSetLayoutDesc({bindings, 4})));
                IDescriptorSetLayout* descriptor_set_layout = m_sdf_descriptor_set_layout;
                luset(m_sdf_pipeline_layout, m_device->new_pipeline_layout(PipelineLayoutDesc(
                    {&descriptor_set_layout, 1}, PipelineLayoutFlag::allow_input_assembler_input_layout)));

                Float2U vertices[] = {
                    Float2U(0.0f, 0.0f), Float2U(0.0f, 1.0f),
                    Float2U(1.0f, 1.0f), Float2U(1.0f, 0.0f)
                };
                u16 indices[] = {0, 1, 2, 0, 2, 3};
                luset(m_sdf_vertex_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(vertices))));
                luset(m_sdf_index_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::index_buffer, sizeof(indices))));
                luset(m_sdf_frame_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::uniform_buffer, sizeof(SDFFrameBuffer))));
                void* mapped_data = nullptr;
                luexp(m_sdf_vertex_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, vertices, sizeof(vertices));
                m_sdf_vertex_buffer->unmap(0, sizeof(vertices));
                luexp(m_sdf_index_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, indices, sizeof(indices));
                m_sdf_index_buffer->unmap(0, sizeof(indices));
            }
            lucatchret;
            return ok;
        }

        RV Renderer::create_sdf_pipeline(RHI::Format render_target_format)
        {
            using namespace RHI;
            lutry
            {
                GraphicsPipelineStateDesc desc;
                InputBindingDesc input_bindings[] = {
                    InputBindingDesc(0, sizeof(Float2U), InputRate::per_vertex),
                    InputBindingDesc(1, sizeof(SDFInstance), InputRate::per_instance)
                };
                InputAttributeDesc input_attributes[] = {
                    InputAttributeDesc(0, 0, 0, Format::rg32_float),
                    InputAttributeDesc(1, 1, offsetof(SDFInstance, draw_rect), Format::rgba32_float),
                    InputAttributeDesc(2, 1, offsetof(SDFInstance, evaluation_origin), Format::rg32_float),
                    InputAttributeDesc(3, 1, offsetof(SDFInstance, program_data), Format::rgba32_uint)
                };
                desc.input_layout = InputLayoutDesc({input_bindings, 2}, {input_attributes, 4});
                desc.pipeline_layout = m_sdf_pipeline_layout;
                desc.vs = LUNA_CPPSL_GET_SHADER_DATA(SDFVS);
                desc.ps = LUNA_CPPSL_GET_SHADER_DATA(SDFPS);
                desc.blend_state = BlendDesc({AttachmentBlendDesc(true,
                    BlendFactor::one, BlendFactor::one_minus_src_alpha, BlendOp::add,
                    BlendFactor::one, BlendFactor::one_minus_src_alpha, BlendOp::add,
                    ColorWriteMask::all)});
                desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::none,
                    false, false, false, false, false);
                desc.depth_stencil_state = DepthStencilDesc(false, false);
                desc.num_color_attachments = 1;
                desc.color_formats[0] = render_target_format;
                luset(m_sdf_pipeline_state, m_device->new_graphics_pipeline_state(desc));
                m_render_target_format = render_target_format;
            }
            lucatchret;
            return ok;
        }

        RV Renderer::compile_draw_commands(IContext* context)
        {
            lutry
            {
                luexp(context->generate_draw_commands());
                const FrameDesc frame_desc = context->get_frame_desc();
                Span<const Layer> layers = context->get_layers();
                Span<const Element> elements = context->get_elements();
                Span<const DrawCommand> commands = context->get_draw_commands();
                Span<const f32> context_shape_floats = context->get_sdf_shape_floats();
                Span<const f32> context_color_floats = context->get_sdf_color_floats();
                m_draw_list->reset();
                m_render_batches.clear();
                m_sdf_instances.clear();
                m_sdf_states.clear();
                m_sdf_page_pairs.clear();
                m_compiled_sdf_shape_floats.clear();
                m_compiled_sdf_color_floats.clear();
                m_compiled_sdf_shape_floats.insert(m_compiled_sdf_shape_floats.end(),
                    context_shape_floats.begin(), context_shape_floats.end());
                m_compiled_sdf_color_floats.insert(m_compiled_sdf_color_floats.end(),
                    context_color_floats.begin(), context_color_floats.end());
                Vector<ClipState> clip_stack;
                RHI::SamplerDesc nearest_sampler_desc;
                nearest_sampler_desc.min_filter = RHI::Filter::nearest;
                nearest_sampler_desc.mag_filter = RHI::Filter::nearest;
                nearest_sampler_desc.mip_filter = RHI::Filter::nearest;
                nearest_sampler_desc.address_u = RHI::TextureAddressMode::clamp;
                nearest_sampler_desc.address_v = RHI::TextureAddressMode::clamp;
                nearest_sampler_desc.address_w = RHI::TextureAddressMode::clamp;
                u32 first_pending_vg_call = 0;

                auto flush_pending_vg = [&]()
                {
                    u32 vg_call_count = (u32)m_draw_list->get_draw_calls().size();
                    if(vg_call_count > first_pending_vg_call)
                    {
                        m_render_batches.push_back({RenderBatchType::vg, first_pending_vg_call,
                            vg_call_count - first_pending_vg_call, 0});
                    }
                    first_pending_vg_call = vg_call_count;
                };

                auto get_page_pair = [&](u32 shape_page, u32 color_page)
                {
                    for(u32 i = 0; i < m_sdf_page_pairs.size(); ++i)
                    {
                        if(m_sdf_page_pairs[i].shape_page == shape_page &&
                            m_sdf_page_pairs[i].color_page == color_page)
                        {
                            return i;
                        }
                    }
                    u32 index = (u32)m_sdf_page_pairs.size();
                    SDFPagePair pair;
                    pair.shape_page = shape_page;
                    pair.color_page = color_page;
                    m_sdf_page_pairs.push_back(move(pair));
                    return index;
                };

                auto resolve_sdf_state = [&](const ClipState& clip)
                {
                    SDFState state;
                    state.clip_rect = Float4U(clip.rect.offset_x, clip.rect.offset_y,
                        clip.rect.width, clip.rect.height);
                    state.rounded_clip_rect = Float4U(clip.rounded_rect.offset_x,
                        clip.rounded_rect.offset_y, clip.rounded_rect.width, clip.rounded_rect.height);
                    state.rounded_clip_radii = clip.rounded_radii;
                    for(u32 i = 0; i < m_sdf_states.size(); ++i)
                    {
                        if(!memcmp(&m_sdf_states[i], &state, sizeof(SDFState))) return i;
                    }
                    u32 state_index = (u32)m_sdf_states.size();
                    m_sdf_states.push_back(state);
                    return state_index;
                };

                auto emit_sdf = [&](const SDFDrawDesc& desc, const Float2U& evaluation_origin,
                    const RectF& raster_domain, const ClipState& clip)
                {
                    const RectF& clip_rect = clip.rect;
                    if(!desc.shape.floats.valid() || !desc.color.floats.valid()) return;
                    u32 shape_page = desc.shape.floats.first_float / SDF_PROGRAM_PAGE_FLOATS;
                    u32 color_page = desc.color.floats.first_float / SDF_PROGRAM_PAGE_FLOATS;
                    u32 local_shape_first = desc.shape.floats.first_float % SDF_PROGRAM_PAGE_FLOATS;
                    u32 local_color_first = desc.color.floats.first_float % SDF_PROGRAM_PAGE_FLOATS;
                    if(local_shape_first + desc.shape.floats.num_floats > SDF_PROGRAM_PAGE_FLOATS ||
                        local_color_first + desc.color.floats.num_floats > SDF_PROGRAM_PAGE_FLOATS)
                    {
                        return;
                    }
                    RectF shape_rect(evaluation_origin.x + desc.shape.bounds.offset_x,
                        evaluation_origin.y + desc.shape.bounds.offset_y,
                        desc.shape.bounds.width, desc.shape.bounds.height);
                    RectF draw_rect(0.0f, 0.0f, 0.0f, 0.0f);
                    if(desc.color.uses_shape_bounds)
                    {
                        draw_rect = expand_rect(shape_rect, desc.color.effect_outsets);
                    }
                    if(desc.color.uses_raster_domain)
                    {
                        RectF domain = rect_visible(raster_domain) ? raster_domain : expand_rect(shape_rect, 1.0f);
                        draw_rect = union_rect(draw_rect, domain);
                    }
                    if(!rect_visible(draw_rect) ||
                        (has_clip(clip_rect) && !rect_visible(intersect_rect(draw_rect, clip_rect))))
                    {
                        return;
                    }
                    SDFInstance instance;
                    instance.draw_rect = Float4U(draw_rect.offset_x, draw_rect.offset_y,
                        draw_rect.width, draw_rect.height);
                    instance.evaluation_origin = evaluation_origin;
                    u32 clip_flags = has_clip(clip_rect) ? 1u : 0u;
                    if(has_rounded_clip(clip)) clip_flags |= 2u;
                    u32 packed_program_data = clip_flags |
                        (desc.color.floats.num_floats << SDF_INSTANCE_COLOR_FLOAT_COUNT_SHIFT);
                    instance.program_data = UInt4U(local_shape_first, local_color_first,
                        packed_program_data, resolve_sdf_state(clip));
                    flush_pending_vg();
                    m_draw_list->draw_call_barrier();
                    u32 pair_index = get_page_pair(shape_page, color_page);
                    u32 instance_index = (u32)m_sdf_instances.size();
                    m_sdf_instances.push_back(instance);
                    if(!m_render_batches.empty() && m_render_batches.back().type == RenderBatchType::sdf &&
                        m_render_batches.back().resource_index == pair_index &&
                        m_render_batches.back().first + m_render_batches.back().count == instance_index)
                    {
                        ++m_render_batches.back().count;
                    }
                    else
                    {
                        m_render_batches.push_back({RenderBatchType::sdf, instance_index, 1, pair_index});
                    }
                };

                for(u32 layer_index = 0; layer_index < layers.size(); ++layer_index)
                {
                    const Layer& layer = layers[layer_index];
                    clip_stack.clear();
                    for(u32 command_index : layer.draw_command_indices)
                    {
                        if(command_index >= commands.size()) continue;
                        const DrawCommand& command = commands[command_index];
                        RectF resolved_rect = resolve_draw_rect(command, elements);
                        RectF element_clip;
                        RectF inherited_clip;
                        if(command.element != INVALID_ELEMENT && command.element < elements.size())
                        {
                            const Element& element = elements[command.element];
                            element_clip = to_screen_rect(layers, layer_index, element.layout_result.clip_rect);
                            if(element.parent != INVALID_ELEMENT && element.parent < elements.size())
                            {
                                inherited_clip = to_screen_rect(layers, layer_index,
                                    elements[element.parent].layout_result.clip_rect);
                            }
                        }
                        if(command.type == DrawCommandType::push_clip)
                        {
                            RectF requested_clip = to_screen_rect(layers, layer_index, resolved_rect);
                            ClipState pushed_clip = clip_stack.empty() ? ClipState() : clip_stack.back();
                            pushed_clip.rect = merge_clip_rect(pushed_clip.rect, element_clip);
                            pushed_clip.rect = merge_clip_rect(pushed_clip.rect, requested_clip);
                            if(command.radius > 0.0f)
                            {
                                pushed_clip.rounded_rect = requested_clip;
                                pushed_clip.rounded_radii = Float4U(command.radius);
                            }
                            clip_stack.push_back(pushed_clip);
                            continue;
                        }
                        if(command.type == DrawCommandType::pop_clip)
                        {
                            if(!clip_stack.empty()) clip_stack.pop_back();
                            continue;
                        }

                        ClipState clip = clip_stack.empty() ? ClipState() : clip_stack.back();
                        clip.rect = merge_clip_rect(clip.rect, element_clip);
                        RectF clip_rect = clip.rect;
                        auto visual_overflow_clip = [&]()
                        {
                            ClipState overflow_clip = clip_stack.empty() ? ClipState() : clip_stack.back();
                            overflow_clip.rect = merge_clip_rect(overflow_clip.rect, inherited_clip);
                            return overflow_clip;
                        };
                        RectF vg_clip = has_clip(clip_rect) ? to_vg_rect(frame_desc, clip_rect) : RectF();
                        m_draw_list->set_clip_rect(vg_clip);
                        RectF vg_rounded_clip = has_rounded_clip(clip) ?
                            to_vg_rect(frame_desc, clip.rounded_rect) : RectF();
                        m_draw_list->set_rounded_clip_rect(vg_rounded_clip, clip.rounded_radii);
                        m_draw_list->set_texture(nullptr);
                        m_draw_list->set_shape_buffer(nullptr);
                        m_draw_list->set_sampler(nullptr);

                        if(command.type == DrawCommandType::rect ||
                            command.type == DrawCommandType::gradient_rect ||
                            command.type == DrawCommandType::rounded_rect ||
                            command.type == DrawCommandType::shadow)
                        {
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if(!rect_visible(screen_rect)) continue;
                            Vector<f32> shape_floats;
                            if(command.type == DrawCommandType::rounded_rect || command.type == DrawCommandType::shadow)
                            {
                                sdf_shape_add_rounded_rectangle(shape_floats,
                                    RectF(0.0f, 0.0f, screen_rect.width, screen_rect.height),
                                    Float4U(max(command.radius, 0.0f)));
                            }
                            else
                            {
                                sdf_shape_add_rectangle(shape_floats,
                                    RectF(0.0f, 0.0f, screen_rect.width, screen_rect.height));
                            }
                            SDFDrawDesc desc;
                            if(!append_shape_program(m_compiled_sdf_shape_floats,
                                shape_floats.cspan(), desc.shape))
                                continue;
                            Vector<f32> color_floats;
                            if(command.type == DrawCommandType::gradient_rect)
                            {
                                sdf_color_add_bilinear_gradient(color_floats,
                                    RectF(0.0f, 0.0f, screen_rect.width, screen_rect.height), command.color,
                                    command.color_top_right, command.color_bottom_right,
                                    command.color_bottom_left);
                            }
                            else if(command.type == DrawCommandType::shadow)
                            {
                                SDFClipDesc shadow_clip = command.shadow.mode == ShadowMode::inner ?
                                    SDFClipDesc::outer(0.0f) : SDFClipDesc::inner(0.0f);
                                sdf_color_add_shadow(color_floats, command.color, command.shadow.offset,
                                    command.shadow.softness, command.shadow.spread, shadow_clip);
                            }
                            else
                            {
                                sdf_color_add_solid(color_floats, command.color);
                            }
                            if(!append_color_program(m_compiled_sdf_color_floats,
                                color_floats.cspan(), desc.color))
                                continue;
                            ClipState sdf_clip = command.type == DrawCommandType::shadow &&
                                command.shadow.mode == ShadowMode::outer ? visual_overflow_clip() : clip;
                            emit_sdf(desc, Float2U(screen_rect.offset_x, screen_rect.offset_y),
                                screen_rect, sdf_clip);
                            continue;
                        }

                        if(command.type == DrawCommandType::sdf)
                        {
                            SDFDrawDesc desc = command.sdf;
                            if(!resolve_context_program(context_shape_floats, command.sdf.shape.floats,
                                desc.shape, validate_sdf_shape_program) ||
                                !resolve_context_program(context_color_floats, command.sdf.color.floats,
                                    desc.color, validate_sdf_color_program))
                            {
                                continue;
                            }
                            RectF screen_anchor = to_screen_rect(layers, layer_index, resolved_rect);
                            ClipState sdf_clip = desc.color.may_paint_outside ?
                                visual_overflow_clip() : clip;
                            emit_sdf(desc, Float2U(screen_anchor.offset_x, screen_anchor.offset_y),
                                screen_anchor, sdf_clip);
                            continue;
                        }

                        switch(command.type)
                        {
                        case DrawCommandType::image:
                        {
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect) || !color_visible(command.color))
                            {
                                break;
                            }
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_texture(command.texture);
                            if(command.nearest_sampler) m_draw_list->set_sampler(&nearest_sampler_desc);
                            auto& points = m_draw_list->get_shape_buffer()->get_shape_points(true);
                            u32 begin = (u32)points.size();
                            VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f,
                                vg_rect.width, vg_rect.height);
                            u32 end = (u32)points.size();
                            m_draw_list->draw_shape(begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height), command.color,
                                command.min_texcoord, command.max_texcoord);
                            break;
                        }
                        case DrawCommandType::line:
                        {
                            if(!color_visible(command.color) || command.line_width <= 0.0f) break;
                            Float2U resolved_p0 = resolve_draw_point0(command, elements);
                            Float2U resolved_p1 = resolve_draw_point1(command, elements);
                            Float2U p0(layer.screen_position.x + resolved_p0.x,
                                layer.screen_position.y + resolved_p0.y);
                            Float2U p1(layer.screen_position.x + resolved_p1.x,
                                layer.screen_position.y + resolved_p1.y);
                            f32 margin = max(command.line_width, 1.0f);
                            f32 dx = abs(p1.x - p0.x);
                            f32 dy = abs(p1.y - p0.y);
                            RectF bounds(min(p0.x, p1.x) - margin, min(p0.y, p1.y) - margin,
                                max(dx + margin * 2.0f, 1.0f), max(dy + margin * 2.0f, 1.0f));
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(bounds, clip_rect))) break;
                            RectF vg_rect = to_vg_rect(frame_desc, bounds);
                            auto& points = m_draw_list->get_shape_buffer()->get_shape_points(true);
                            u32 begin = (u32)points.size();
                            Float2U local_p0(p0.x - bounds.offset_x, bounds.height - (p0.y - bounds.offset_y));
                            Float2U local_p1(p1.x - bounds.offset_x, bounds.height - (p1.y - bounds.offset_y));
                            VG::ShapeBuilder::add_line(points, local_p0.x, local_p0.y,
                                local_p1.x, local_p1.y, command.line_width);
                            u32 end = (u32)points.size();
                            m_draw_list->draw_shape(begin, end - begin,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height), command.color,
                                Float2U(0.0f), Float2U(1.0f));
                            break;
                        }
                        case DrawCommandType::text:
                        {
                            if(command.text.empty() || command.font_size <= 0.0f || !color_visible(command.color))
                                break;
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect)) break;
                            FontDesc font = context->get_font(command.font);
                            if(!font.font)
                            {
                                font.font = Font::get_default_font();
                                font.font_index = 0;
                            }
                            VG::TextArrangeSection section;
                            section.font_file = font.font;
                            section.font_index = font.font_index;
                            section.font_size = command.font_size;
                            section.color = command.color;
                            section.num_chars = command.text.size();
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            VG::TextArrangeResult arranged = VG::arrange_text(command.text.c_str(), command.text.size(),
                                Span<const VG::TextArrangeSection>(&section, 1), vg_rect,
                                command.vertical_alignment, command.horizontal_alignment);
                            VG::commit_text_arrange_result(arranged,
                                Span<const VG::TextArrangeSection>(&section, 1), m_font_atlas,
                                m_draw_list);
                            break;
                        }
                        case DrawCommandType::shape:
                        {
                            if(!command.shape.buffer || command.shape.num_commands == 0 ||
                                !rect_visible(command.shape.bounds) || !rect_visible(resolved_rect) ||
                                !color_visible(command.color)) break;
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) break;
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_shape_buffer(command.shape.buffer);
                            m_draw_list->set_texture(command.shape.texture);
                            if(command.shape.texture && command.nearest_sampler)
                                m_draw_list->set_sampler(&nearest_sampler_desc);
                            Float2U shape_min(command.shape.bounds.offset_x,
                                command.shape.bounds.offset_y + command.shape.bounds.height);
                            Float2U shape_max(command.shape.bounds.offset_x + command.shape.bounds.width,
                                command.shape.bounds.offset_y);
                            m_draw_list->draw_shape(command.shape.first_command, command.shape.num_commands,
                                Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                shape_min, shape_max, command.color,
                                command.min_texcoord, command.max_texcoord);
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
                m_draw_list->set_texture(nullptr);
                m_draw_list->set_shape_buffer(nullptr);
                m_draw_list->set_sampler(nullptr);
                m_draw_list->set_clip_rect(RectF());
                m_draw_list->set_rounded_clip_rect(RectF(), Float4U(0.0f));
                flush_pending_vg();
                luexp(m_draw_list->compile());
                m_counters.vg_draw_call_count = (u32)m_draw_list->get_draw_calls().size();
                m_counters.sdf_instance_count = (u32)m_sdf_instances.size();
                m_counters.sdf_state_count = (u32)m_sdf_states.size();
                m_counters.sdf_shape_float_count = (u32)m_compiled_sdf_shape_floats.size();
                m_counters.sdf_color_float_count = (u32)m_compiled_sdf_color_floats.size();
                m_counters.sdf_program_page_count =
                    (u32)((m_compiled_sdf_shape_floats.size() + SDF_PROGRAM_PAGE_FLOATS - 1) /
                        SDF_PROGRAM_PAGE_FLOATS) +
                    (u32)((m_compiled_sdf_color_floats.size() + SDF_PROGRAM_PAGE_FLOATS - 1) /
                        SDF_PROGRAM_PAGE_FLOATS);
                m_counters.sdf_page_pair_count = (u32)m_sdf_page_pairs.size();
                m_counters.sdf_instance_upload_bytes = m_sdf_instances.size() * sizeof(SDFInstance);
                m_counters.sdf_state_upload_bytes = m_sdf_states.size() * sizeof(SDFState);
                m_counters.sdf_upload_bytes = m_counters.sdf_instance_upload_bytes +
                    m_counters.sdf_state_upload_bytes +
                    (m_compiled_sdf_shape_floats.size() + m_compiled_sdf_color_floats.size()) * sizeof(f32);
                m_counters.sdf_draw_call_count = 0;
                for(const RenderBatch& batch : m_render_batches)
                {
                    if(batch.type == RenderBatchType::sdf) ++m_counters.sdf_draw_call_count;
                }
                m_counters.backend_switch_count = 0;
                for(usize i = 1; i < m_render_batches.size(); ++i)
                {
                    if(m_render_batches[i - 1].type != m_render_batches[i].type)
                        ++m_counters.backend_switch_count;
                }
                m_counters.render_batch_count = (u32)m_render_batches.size();
            }
            lucatchret;
            return ok;
        }

        RV Renderer::prepare_sdf_resources()
        {
            using namespace RHI;
            if(m_sdf_instances.empty()) return ok;
            if(m_sdf_states.empty())
                return set_error(BasicError::bad_data(), "SDF instances were compiled without raster states.");
            lutry
            {
                SDFFrameBuffer frame_data;
                frame_data.screen_params = Float4U(m_screen_width, m_screen_height,
                    1.0f / m_screen_width, 1.0f / m_screen_height);
                void* mapped_data = nullptr;
                luexp(m_sdf_frame_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, &frame_data, sizeof(frame_data));
                m_sdf_frame_buffer->unmap(0, sizeof(frame_data));

                if(m_sdf_instance_capacity < m_sdf_instances.size())
                {
                    luset(m_sdf_instance_buffer, m_device->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::vertex_buffer,
                            sizeof(SDFInstance) * m_sdf_instances.size())));
                    m_sdf_instance_capacity = m_sdf_instances.size();
                }
                usize instance_bytes = sizeof(SDFInstance) * m_sdf_instances.size();
                luexp(m_sdf_instance_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, m_sdf_instances.data(), instance_bytes);
                m_sdf_instance_buffer->unmap(0, instance_bytes);

                if(m_sdf_state_capacity < m_sdf_states.size())
                {
                    luset(m_sdf_state_buffer, m_device->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::read_buffer,
                            sizeof(SDFState) * m_sdf_states.size())));
                    m_sdf_state_capacity = m_sdf_states.size();
                }
                usize state_bytes = sizeof(SDFState) * m_sdf_states.size();
                luexp(m_sdf_state_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, m_sdf_states.data(), state_bytes);
                m_sdf_state_buffer->unmap(0, state_bytes);

                auto upload_pages = [&](const Vector<f32>& floats, Vector<SDFProgramPage>& pages) -> RV
                {
                    usize num_pages = (floats.size() + SDF_PROGRAM_PAGE_FLOATS - 1) /
                        SDF_PROGRAM_PAGE_FLOATS;
                    pages.resize(num_pages);
                    for(usize page_index = 0; page_index < num_pages; ++page_index)
                    {
                        usize first_float = page_index * SDF_PROGRAM_PAGE_FLOATS;
                        u32 num_floats = (u32)min<usize>(SDF_PROGRAM_PAGE_FLOATS,
                            floats.size() - first_float);
                        usize num_bytes = (usize)num_floats * sizeof(f32);
                        SDFProgramPage& page = pages[page_index];
                        if(!page.buffer || page.buffer->get_desc().size < num_bytes)
                        {
                            auto buffer_result = m_device->new_buffer(MemoryType::upload,
                                BufferDesc(BufferUsageFlag::read_buffer, num_bytes));
                            if(failed(buffer_result)) return buffer_result.errcode();
                            page.buffer = buffer_result.get();
                        }
                        page.num_floats = num_floats;
                        void* page_data = nullptr;
                        RV map_result = page.buffer->map(0, 0, &page_data);
                        if(failed(map_result)) return map_result;
                        memcpy(page_data, floats.data() + first_float, num_bytes);
                        page.buffer->unmap(0, num_bytes);
                    }
                    return ok;
                };
                luexp(upload_pages(m_compiled_sdf_shape_floats, m_sdf_shape_pages));
                luexp(upload_pages(m_compiled_sdf_color_floats, m_sdf_color_pages));

                for(SDFPagePair& pair : m_sdf_page_pairs)
                {
                    if(pair.shape_page >= m_sdf_shape_pages.size() ||
                        pair.color_page >= m_sdf_color_pages.size())
                    {
                        return set_error(BasicError::bad_data(), "SDF draw references an unavailable program page.");
                    }
                    lulet(descriptor_set, m_device->new_descriptor_set(
                        RHI::DescriptorSetDesc(m_sdf_descriptor_set_layout)));
                    pair.descriptor_set = descriptor_set;
                    SDFProgramPage& shape_page = m_sdf_shape_pages[pair.shape_page];
                    SDFProgramPage& color_page = m_sdf_color_pages[pair.color_page];
                    luexp(pair.descriptor_set->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0,
                            BufferViewDesc::uniform_buffer(m_sdf_frame_buffer, 0, sizeof(SDFFrameBuffer))),
                        WriteDescriptorSet::read_buffer_view(1,
                            BufferViewDesc::structured_buffer(shape_page.buffer, 0,
                                shape_page.num_floats, sizeof(f32))),
                        WriteDescriptorSet::read_buffer_view(2,
                            BufferViewDesc::structured_buffer(color_page.buffer, 0,
                                color_page.num_floats, sizeof(f32))),
                        WriteDescriptorSet::read_buffer_view(3,
                            BufferViewDesc::structured_buffer(m_sdf_state_buffer, 0,
                                (u32)m_sdf_states.size(), sizeof(SDFState)))
                    }));
                }
            }
            lucatchret;
            return ok;
        }

        void Renderer::render_sdf(RHI::ICommandBuffer* cmdbuf, const RenderBatch& batch)
        {
            using namespace RHI;
            if(batch.resource_index >= m_sdf_page_pairs.size() || !batch.count) return;
            cmdbuf->set_graphics_pipeline_state(m_sdf_pipeline_state);
            cmdbuf->set_graphics_pipeline_layout(m_sdf_pipeline_layout);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)m_render_target_width,
                (f32)m_render_target_height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, m_render_target_width, m_render_target_height));
            cmdbuf->set_graphics_descriptor_set(0,
                m_sdf_page_pairs[batch.resource_index].descriptor_set);
            VertexBufferView vertex_views[] = {
                VertexBufferView(m_sdf_vertex_buffer, 0, sizeof(Float2U) * 4, sizeof(Float2U)),
                VertexBufferView(m_sdf_instance_buffer, 0,
                    sizeof(SDFInstance) * m_sdf_instances.size(), sizeof(SDFInstance))
            };
            cmdbuf->set_vertex_buffers(0, {vertex_views, 2});
            cmdbuf->set_index_buffer(IndexBufferView(m_sdf_index_buffer, 0,
                sizeof(u16) * 6, Format::r16_uint));
            cmdbuf->draw_indexed_instanced(6, batch.count, 0, 0, batch.first);
        }

        RV Renderer::prepare(IContext* context, RHI::ICommandBuffer* cmdbuf,
            RHI::ITexture* render_target)
        {
            if(!context || !cmdbuf || !render_target) return BasicError::bad_arguments();
            auto render_target_desc = render_target->get_desc();
            if(render_target_desc.format != m_render_target_format)
            {
                RV result = create_sdf_pipeline(render_target_desc.format);
                if(failed(result)) return result;
            }
            m_render_target_width = render_target_desc.width;
            m_render_target_height = render_target_desc.height;
            const FrameDesc frame_desc = context->get_frame_desc();
            m_screen_width = max(frame_desc.screen_size.x, 1.0f);
            m_screen_height = max(frame_desc.screen_size.y, 1.0f);
            m_counters = RendererPerformanceCounters();
            u64 prepare_begin = get_ticks();
            lutry
            {
                luexp(compile_draw_commands(context));
                luexp(m_shape_renderer->begin(render_target));
                Float4x4U transform = ProjectionMatrix::make_orthographic_off_center(
                    0.0f, m_screen_width, 0.0f, m_screen_height, 0.0f, 1.0f);
                m_shape_renderer->draw(m_draw_list->get_instance_buffer(),
                    m_draw_list->get_state_buffer(),
                    m_draw_list->get_draw_calls(), &transform);
                luexp(m_shape_renderer->end());
                luexp(prepare_sdf_resources());
                if(!m_sdf_instances.empty())
                {
                    Vector<RHI::BufferBarrier> barriers;
                    barriers.push_back(RHI::BufferBarrier(m_sdf_vertex_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::vertex_buffer));
                    barriers.push_back(RHI::BufferBarrier(m_sdf_index_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::index_buffer));
                    barriers.push_back(RHI::BufferBarrier(m_sdf_instance_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::vertex_buffer));
                    barriers.push_back(RHI::BufferBarrier(m_sdf_state_buffer,
                        RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::shader_read_vs));
                    for(const SDFProgramPage& page : m_sdf_shape_pages)
                    {
                        barriers.push_back(RHI::BufferBarrier(page.buffer,
                            RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::shader_read_ps));
                    }
                    for(const SDFProgramPage& page : m_sdf_color_pages)
                    {
                        barriers.push_back(RHI::BufferBarrier(page.buffer,
                            RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::shader_read_ps));
                    }
                    cmdbuf->resource_barrier(barriers.cspan(), {});
                }
                m_shape_renderer->prepare(cmdbuf);
            }
            lucatchret;
            m_counters.prepare_ms = perf_elapsed_ms(prepare_begin, get_ticks());
            return ok;
        }

        void Renderer::render(RHI::ICommandBuffer* cmdbuf)
        {
            if(!cmdbuf) return;
            for(const RenderBatch& batch : m_render_batches)
            {
                if(batch.type == RenderBatchType::vg)
                {
                    m_shape_renderer->submit(cmdbuf, batch.first, batch.count);
                }
                else
                {
                    render_sdf(cmdbuf, batch);
                }
            }
        }

        RendererPerformanceCounters Renderer::get_performance_counters() const
        {
            return m_counters;
        }

        LUNA_GUICORE_API R<Ref<IRenderer>> new_renderer(RHI::IDevice* device)
        {
            Ref<Renderer> renderer = new_object<Renderer>();
            RV result = renderer->init(device);
            if(failed(result)) return result.errcode();
            Ref<IRenderer> ret = renderer;
            return ret;
        }
    }
}
