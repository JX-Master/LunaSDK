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
#include <ShadowVS.hpp>
#include <ShadowPS.hpp>

namespace Luna
{
    namespace GUICore
    {
        namespace
        {
            struct ShadowConstantBuffer
            {
                Float4U draw_rect;
                Float4U source_rect;
                Float4U clip_rect;
                Float4U color;
                Float4U shadow_params;
                Float4U screen_params;
            };

            bool color_visible(const Float4U& color)
            {
                return color.w > 0.0f;
            }

            bool command_color_visible(const DrawCommand& command)
            {
                if(command.type == DrawCommandType::gradient_rect)
                {
                    return color_visible(command.color) ||
                        color_visible(command.color_top_right) ||
                        color_visible(command.color_bottom_right) ||
                        color_visible(command.color_bottom_left);
                }
                return color_visible(command.color);
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

            bool has_clip(const RectF& clip_rect)
            {
                return clip_rect.width > 0.0f || clip_rect.height > 0.0f;
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
                    max(element_rect.width - command.rect.offset_x + command.rect.width, 1.0f);
                ret.height = scaled_height > 0.0f ? scaled_height :
                    max(element_rect.height - command.rect.offset_y + command.rect.height, 1.0f);
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
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all)
                };
                luset(m_shadow_descriptor_set_layout,
                    m_device->new_descriptor_set_layout(DescriptorSetLayoutDesc({bindings, 1})));
                IDescriptorSetLayout* descriptor_set_layout = m_shadow_descriptor_set_layout;
                luset(m_shadow_pipeline_layout, m_device->new_pipeline_layout(PipelineLayoutDesc(
                    {&descriptor_set_layout, 1}, PipelineLayoutFlag::allow_input_assembler_input_layout)));

                Float2U vertices[] = {
                    Float2U(0.0f, 0.0f), Float2U(0.0f, 1.0f),
                    Float2U(1.0f, 1.0f), Float2U(1.0f, 0.0f)
                };
                u16 indices[] = {0, 1, 2, 0, 2, 3};
                luset(m_shadow_vertex_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(vertices))));
                luset(m_shadow_index_buffer, m_device->new_buffer(MemoryType::upload,
                    BufferDesc(BufferUsageFlag::index_buffer, sizeof(indices))));
                void* mapped_data = nullptr;
                luexp(m_shadow_vertex_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, vertices, sizeof(vertices));
                m_shadow_vertex_buffer->unmap(0, sizeof(vertices));
                luexp(m_shadow_index_buffer->map(0, 0, &mapped_data));
                memcpy(mapped_data, indices, sizeof(indices));
                m_shadow_index_buffer->unmap(0, sizeof(indices));
                m_shadow_constant_buffer_stride = (u32)align_upper(sizeof(ShadowConstantBuffer),
                    m_device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment);
            }
            lucatchret;
            return ok;
        }

        RV Renderer::create_shadow_pipeline(RHI::Format render_target_format)
        {
            using namespace RHI;
            lutry
            {
                GraphicsPipelineStateDesc desc;
                InputBindingDesc input_bindings[] = {
                    InputBindingDesc(0, sizeof(Float2U), InputRate::per_vertex)
                };
                InputAttributeDesc input_attributes[] = {
                    InputAttributeDesc(0, 0, 0, Format::rg32_float)
                };
                desc.input_layout = InputLayoutDesc({input_bindings, 1}, {input_attributes, 1});
                desc.pipeline_layout = m_shadow_pipeline_layout;
                desc.vs = LUNA_CPPSL_GET_SHADER_DATA(ShadowVS);
                desc.ps = LUNA_CPPSL_GET_SHADER_DATA(ShadowPS);
                desc.blend_state = BlendDesc({AttachmentBlendDesc(true,
                    BlendFactor::src_alpha, BlendFactor::one_minus_src_alpha, BlendOp::add,
                    BlendFactor::zero, BlendFactor::one, BlendOp::add, ColorWriteMask::all)});
                desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::none,
                    false, false, false, false, false);
                desc.depth_stencil_state = DepthStencilDesc(false, false);
                desc.num_color_attachments = 1;
                desc.color_formats[0] = render_target_format;
                luset(m_shadow_pipeline_state, m_device->new_graphics_pipeline_state(desc));
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
                m_draw_list->reset();
                m_render_batches.clear();
                m_shadow_calls.clear();
                Vector<RectF> clip_stack;
                RHI::SamplerDesc nearest_sampler_desc;
                nearest_sampler_desc.min_filter = RHI::Filter::nearest;
                nearest_sampler_desc.mag_filter = RHI::Filter::nearest;
                nearest_sampler_desc.mip_filter = RHI::Filter::nearest;
                nearest_sampler_desc.address_u = RHI::TextureAddressMode::clamp;
                nearest_sampler_desc.address_v = RHI::TextureAddressMode::clamp;
                nearest_sampler_desc.address_w = RHI::TextureAddressMode::clamp;
                u32 first_pending_vg_call = 0;

                for(u32 layer_index = 0; layer_index < layers.size(); ++layer_index)
                {
                    const Layer& layer = layers[layer_index];
                    clip_stack.clear();
                    for(u32 command_index : layer.draw_command_indices)
                    {
                        if(command_index >= commands.size())
                        {
                            continue;
                        }
                        const DrawCommand& command = commands[command_index];
                        RectF resolved_rect = resolve_draw_rect(command, elements);
                        RectF element_clip;
                        if(command.element != INVALID_ELEMENT && command.element < elements.size())
                        {
                            element_clip = to_screen_rect(layers, layer_index,
                                elements[command.element].layout_result.clip_rect);
                        }
                        if(command.type == DrawCommandType::push_clip)
                        {
                            RectF screen_clip = to_screen_rect(layers, layer_index, resolved_rect);
                            screen_clip = merge_clip_rect(screen_clip, element_clip);
                            screen_clip = clip_stack.empty() ? screen_clip : intersect_rect(clip_stack.back(), screen_clip);
                            clip_stack.push_back(screen_clip);
                            continue;
                        }
                        if(command.type == DrawCommandType::pop_clip)
                        {
                            if(!clip_stack.empty())
                            {
                                clip_stack.pop_back();
                            }
                            continue;
                        }

                        RectF clip_rect = clip_stack.empty() ? element_clip :
                            merge_clip_rect(clip_stack.back(), element_clip);
                        RectF vg_clip = has_clip(clip_rect) ? to_vg_rect(frame_desc, clip_rect) : RectF();
                        m_draw_list->set_clip_rect(vg_clip);
                        m_draw_list->set_texture(nullptr);
                        m_draw_list->set_shape_buffer(nullptr);
                        m_draw_list->set_sampler(nullptr);

                        if(command.type == DrawCommandType::shadow)
                        {
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if(!rect_visible(screen_rect) || !color_visible(command.color))
                            {
                                continue;
                            }
                            f32 spread = command.shadow.spread;
                            RectF source_rect(screen_rect.offset_x + command.shadow.offset.x - spread,
                                screen_rect.offset_y + command.shadow.offset.y - spread,
                                max(screen_rect.width + spread * 2.0f, 1.0f),
                                max(screen_rect.height + spread * 2.0f, 1.0f));
                            f32 radius = max(command.radius + spread, 0.0f);
                            f32 softness = max(command.shadow.softness, 0.0f);
                            f32 margin = max(softness * 3.0f + 1.0f, 1.0f);
                            RectF draw_rect = command.shadow.mode == ShadowMode::outer ?
                                RectF(source_rect.offset_x - margin, source_rect.offset_y - margin,
                                    source_rect.width + margin * 2.0f, source_rect.height + margin * 2.0f) :
                                screen_rect;
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(draw_rect, clip_rect)))
                            {
                                continue;
                            }
                            u32 vg_call_count = (u32)m_draw_list->get_draw_calls().size();
                            if(vg_call_count > first_pending_vg_call)
                            {
                                m_render_batches.push_back({RenderBatchType::vg, first_pending_vg_call,
                                    vg_call_count - first_pending_vg_call});
                            }
                            ShadowCall shadow_call;
                            shadow_call.draw_rect = draw_rect;
                            shadow_call.source_rect = source_rect;
                            shadow_call.clip_rect = clip_rect;
                            shadow_call.color = command.color;
                            shadow_call.radius = radius;
                            shadow_call.source_radius = max(command.radius, 0.0f);
                            shadow_call.softness = softness;
                            shadow_call.mode = command.shadow.mode;
                            u32 shadow_index = (u32)m_shadow_calls.size();
                            m_shadow_calls.push_back(shadow_call);
                            m_render_batches.push_back({RenderBatchType::shadow, shadow_index, 1});
                            m_draw_list->draw_call_barrier();
                            first_pending_vg_call = vg_call_count;
                            continue;
                        }

                        switch(command.type)
                        {
                        case DrawCommandType::rect:
                        case DrawCommandType::gradient_rect:
                        case DrawCommandType::rounded_rect:
                        case DrawCommandType::image:
                        {
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect) || !command_color_visible(command))
                            {
                                break;
                            }
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_texture(command.type == DrawCommandType::image ? command.texture : nullptr);
                            if(command.type == DrawCommandType::image && command.nearest_sampler)
                            {
                                m_draw_list->set_sampler(&nearest_sampler_desc);
                            }
                            auto& points = m_draw_list->get_shape_buffer()->get_shape_points(true);
                            u32 begin = (u32)points.size();
                            if(command.type == DrawCommandType::rounded_rect && command.radius > 0.0f)
                            {
                                VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0.0f, 0.0f,
                                    vg_rect.width, vg_rect.height,
                                    min(command.radius, min(vg_rect.width, vg_rect.height) * 0.5f));
                            }
                            else
                            {
                                VG::ShapeBuilder::add_rectangle_filled(points, 0.0f, 0.0f,
                                    vg_rect.width, vg_rect.height);
                            }
                            u32 end = (u32)points.size();
                            if(command.type == DrawCommandType::gradient_rect)
                            {
                                VG::Vertex vertices[4];
                                u32 indices[6];
                                VG::get_rect_shape_draw_vertices(vertices, indices, begin, end - begin,
                                    Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                    Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                    Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height),
                                    command.color, command.min_texcoord, command.max_texcoord);
                                vertices[0].color = command.color_bottom_left;
                                vertices[1].color = command.color;
                                vertices[2].color = command.color_top_right;
                                vertices[3].color = command.color_bottom_right;
                                m_draw_list->draw_shape_raw(Span<const VG::Vertex>(vertices, 4),
                                    Span<const u32>(indices, 6));
                            }
                            else
                            {
                                m_draw_list->draw_shape(begin, end - begin,
                                    Float2U(vg_rect.offset_x, vg_rect.offset_y),
                                    Float2U(vg_rect.offset_x + vg_rect.width, vg_rect.offset_y + vg_rect.height),
                                    Float2U(0.0f), Float2U(vg_rect.width, vg_rect.height), command.color,
                                    command.min_texcoord, command.max_texcoord);
                            }
                            break;
                        }
                        case DrawCommandType::line:
                        {
                            if(!color_visible(command.color) || command.line_width <= 0.0f)
                            {
                                break;
                            }
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
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(bounds, clip_rect)))
                            {
                                break;
                            }
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
                            {
                                break;
                            }
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if((has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect))) ||
                                !rect_visible(screen_rect))
                            {
                                break;
                            }
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
                            Vector<VG::Vertex> vertices;
                            Vector<u32> indices;
                            VG::generate_text_arrange_result_draw_vertices(arranged,
                                Span<const VG::TextArrangeSection>(&section, 1), m_font_atlas,
                                vertices, indices);
                            m_draw_list->set_shape_buffer(m_font_atlas->get_shape_buffer());
                            m_draw_list->set_texture(nullptr);
                            m_draw_list->draw_shape_raw(vertices.cspan(), indices.cspan());
                            break;
                        }
                        case DrawCommandType::shape:
                        {
                            if(!command.shape.buffer || command.shape.num_commands == 0 ||
                                !rect_visible(command.shape.bounds) || !rect_visible(resolved_rect) ||
                                !color_visible(command.color))
                            {
                                break;
                            }
                            RectF screen_rect = to_screen_rect(layers, layer_index, resolved_rect);
                            if(has_clip(clip_rect) && !rect_visible(intersect_rect(screen_rect, clip_rect)))
                            {
                                break;
                            }
                            RectF vg_rect = to_vg_rect(frame_desc, screen_rect);
                            m_draw_list->set_shape_buffer(command.shape.buffer);
                            m_draw_list->set_texture(command.shape.texture);
                            if(command.shape.texture && command.nearest_sampler)
                            {
                                m_draw_list->set_sampler(&nearest_sampler_desc);
                            }
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
                u32 vg_call_count = (u32)m_draw_list->get_draw_calls().size();
                if(vg_call_count > first_pending_vg_call)
                {
                    m_render_batches.push_back({RenderBatchType::vg, first_pending_vg_call,
                        vg_call_count - first_pending_vg_call});
                }
                luexp(m_draw_list->compile());
                m_counters.vg_draw_call_count = (u32)m_draw_list->get_draw_calls().size();
                m_counters.shadow_draw_call_count = (u32)m_shadow_calls.size();
                m_counters.render_batch_count = (u32)m_render_batches.size();
            }
            lucatchret;
            return ok;
        }

        RV Renderer::prepare_shadow_resources()
        {
            using namespace RHI;
            if(m_shadow_calls.empty())
            {
                return ok;
            }
            usize required_size = (usize)m_shadow_constant_buffer_stride * m_shadow_calls.size();
            lutry
            {
                if(m_shadow_constant_buffer_capacity < m_shadow_calls.size())
                {
                    luset(m_shadow_constant_buffer, m_device->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::uniform_buffer, required_size)));
                    m_shadow_constant_buffer_capacity = m_shadow_calls.size();
                }
                while(m_shadow_descriptor_sets.size() < m_shadow_calls.size())
                {
                    lulet(descriptor_set, m_device->new_descriptor_set(
                        DescriptorSetDesc(m_shadow_descriptor_set_layout)));
                    m_shadow_descriptor_sets.push_back(descriptor_set);
                }
                void* mapped_data = nullptr;
                luexp(m_shadow_constant_buffer->map(0, 0, &mapped_data));
                for(usize i = 0; i < m_shadow_calls.size(); ++i)
                {
                    const ShadowCall& call = m_shadow_calls[i];
                    ShadowConstantBuffer* data = (ShadowConstantBuffer*)((usize)mapped_data +
                        i * m_shadow_constant_buffer_stride);
                    data->draw_rect = Float4U(call.draw_rect.offset_x, call.draw_rect.offset_y,
                        call.draw_rect.width, call.draw_rect.height);
                    data->source_rect = Float4U(call.source_rect.offset_x, call.source_rect.offset_y,
                        call.source_rect.width, call.source_rect.height);
                    data->clip_rect = Float4U(call.clip_rect.offset_x, call.clip_rect.offset_y,
                        call.clip_rect.width, call.clip_rect.height);
                    data->color = call.color;
                    data->shadow_params = Float4U(call.radius, call.softness,
                        call.mode == ShadowMode::inner ? 1.0f : 0.0f,
                        has_clip(call.clip_rect) ? 1.0f : 0.0f);
                    data->screen_params = Float4U(m_screen_width, m_screen_height,
                        call.source_radius, 0.0f);
                    luexp(m_shadow_descriptor_sets[i]->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0,
                            BufferViewDesc::uniform_buffer(m_shadow_constant_buffer,
                                i * m_shadow_constant_buffer_stride, sizeof(ShadowConstantBuffer)))
                    }));
                }
                m_shadow_constant_buffer->unmap(0, required_size);
            }
            lucatchret;
            return ok;
        }

        void Renderer::render_shadow(RHI::ICommandBuffer* cmdbuf, u32 shadow_index)
        {
            using namespace RHI;
            if(shadow_index >= m_shadow_calls.size())
            {
                return;
            }
            cmdbuf->set_graphics_pipeline_state(m_shadow_pipeline_state);
            cmdbuf->set_graphics_pipeline_layout(m_shadow_pipeline_layout);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)m_render_target_width,
                (f32)m_render_target_height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, m_render_target_width, m_render_target_height));
            cmdbuf->set_graphics_descriptor_set(0, m_shadow_descriptor_sets[shadow_index]);
            cmdbuf->set_vertex_buffers(0, {VertexBufferView(m_shadow_vertex_buffer, 0,
                sizeof(Float2U) * 4, sizeof(Float2U))});
            cmdbuf->set_index_buffer(IndexBufferView(m_shadow_index_buffer, 0,
                sizeof(u16) * 6, Format::r16_uint));
            cmdbuf->draw_indexed(6, 0, 0);
        }

        RV Renderer::prepare(IContext* context, RHI::ICommandBuffer* cmdbuf,
            RHI::ITexture* render_target)
        {
            if(!context || !cmdbuf || !render_target)
            {
                return BasicError::bad_arguments();
            }
            auto render_target_desc = render_target->get_desc();
            if(render_target_desc.format != m_render_target_format)
            {
                RV result = create_shadow_pipeline(render_target_desc.format);
                if(failed(result))
                {
                    return result;
                }
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
                m_shape_renderer->draw(m_draw_list->get_vertex_buffer(), m_draw_list->get_index_buffer(),
                    m_draw_list->get_draw_calls(), &transform);
                luexp(m_shape_renderer->end());
                luexp(prepare_shadow_resources());
                if(!m_shadow_calls.empty())
                {
                    cmdbuf->resource_barrier({
                        RHI::BufferBarrier(m_shadow_vertex_buffer, RHI::BufferStateFlag::automatic,
                            RHI::BufferStateFlag::vertex_buffer),
                        RHI::BufferBarrier(m_shadow_index_buffer, RHI::BufferStateFlag::automatic,
                            RHI::BufferStateFlag::index_buffer)
                    }, {});
                }
                m_shape_renderer->prepare(cmdbuf);
            }
            lucatchret;
            m_counters.prepare_ms = perf_elapsed_ms(prepare_begin, get_ticks());
            return ok;
        }

        void Renderer::render(RHI::ICommandBuffer* cmdbuf)
        {
            if(!cmdbuf)
            {
                return;
            }
            for(const RenderBatch& batch : m_render_batches)
            {
                if(batch.type == RenderBatchType::vg)
                {
                    m_shape_renderer->submit(cmdbuf, batch.first, batch.count);
                }
                else
                {
                    for(u32 i = 0; i < batch.count; ++i)
                    {
                        render_shadow(cmdbuf, batch.first + i);
                    }
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
            if(failed(result))
            {
                return result.errcode();
            }
            Ref<IRenderer> ret = renderer;
            return ret;
        }
    }
}
