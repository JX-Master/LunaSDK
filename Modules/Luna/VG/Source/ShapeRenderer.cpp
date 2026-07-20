/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeRenderer.cpp
* @author JXMaster
* @date 2022/4/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "ShapeRendererImpl.hpp"
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <FillVS.hpp>
#include <FillPS.hpp>

namespace Luna
{
    namespace VG
    {
        Ref<RHI::IDescriptorSetLayout> g_fill_desc_layout;
        Ref<RHI::IPipelineLayout> g_fill_playout;
        Ref<RHI::ITexture> g_white_tex;
        Ref<RHI::IBuffer> g_quad_vertex_buffer;
        Ref<RHI::IBuffer> g_quad_index_buffer;

        RV init_render_resources()
        {
            using namespace RHI;
            auto dev = get_main_device();
            lutry
            {
                {
                    DescriptorSetLayoutBinding bindings[] = {
                        DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding::read_buffer_view(1, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::pixel),
                        DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::pixel),
                        DescriptorSetLayoutBinding::read_buffer_view(4, 1, ShaderVisibilityFlag::vertex)
                    };
                    DescriptorSetLayoutDesc desc({bindings, 5});
                    luset(g_fill_desc_layout, dev->new_descriptor_set_layout(desc));
                }
                {
                    IDescriptorSetLayout* dl = g_fill_desc_layout;
                    PipelineLayoutDesc desc ({&dl, 1},
                        PipelineLayoutFlag::allow_input_assembler_input_layout
                    );
                    luset(g_fill_playout, dev->new_pipeline_layout(desc));
                }
                {
                    TextureDesc desc = TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1);
                    luset(g_white_tex, dev->new_texture(MemoryType::local, desc));
                    u32 data = 0xFFFFFFFF;
                    {
                        u32 copy_queue_index = U32_MAX;
                        {
                            // Prefer a dedicated copy queue if present.
                            u32 num_queues = dev->get_num_command_queues();
                            for (u32 i = 0; i < num_queues; ++i)
                            {
                                auto desc = dev->get_command_queue_desc(i);
                                if (desc.type == CommandQueueType::graphics && copy_queue_index == U32_MAX)
                                {
                                    copy_queue_index = i;
                                }
                                else if (desc.type == CommandQueueType::copy)
                                {
                                    copy_queue_index = i;
                                    break;
                                }
                            }
                        }
                        lulet(upload_cmdbuf, dev->new_command_buffer(copy_queue_index));
                        auto writer = RHIUtility::new_resource_write_context(dev);
                        u32 row_pitch, slice_pitch;
                        lulet(mapped, writer->write_texture(g_white_tex, SubresourceIndex(0, 0), 0, 0, 0, 1, 1, 1, row_pitch, slice_pitch));
                        memcpy_bitmap(mapped, &data, sizeof(data), 1, row_pitch, sizeof(data));
                        luexp(writer->commit(upload_cmdbuf, true));
                    }
                }
                {
                    Float2U vertices[] = {
                        Float2U(0.0f, 0.0f), Float2U(0.0f, 1.0f),
                        Float2U(1.0f, 1.0f), Float2U(1.0f, 0.0f)
                    };
                    u16 indices[] = {0, 1, 2, 0, 2, 3};
                    luset(g_quad_vertex_buffer, dev->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(vertices))));
                    luset(g_quad_index_buffer, dev->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::index_buffer, sizeof(indices))));
                    void* mapped_data = nullptr;
                    luexp(g_quad_vertex_buffer->map(0, 0, &mapped_data));
                    memcpy(mapped_data, vertices, sizeof(vertices));
                    g_quad_vertex_buffer->unmap(0, sizeof(vertices));
                    luexp(g_quad_index_buffer->map(0, 0, &mapped_data));
                    memcpy(mapped_data, indices, sizeof(indices));
                    g_quad_index_buffer->unmap(0, sizeof(indices));
                }
            }
            lucatchret;
            return ok;
        }
        void deinit_render_resources()
        {
            g_fill_desc_layout = nullptr;
            g_fill_playout = nullptr;
            g_white_tex = nullptr;
            g_quad_vertex_buffer = nullptr;
            g_quad_index_buffer = nullptr;
        }
        RV FillShapeRenderer::create_pso(RHI::Format rt_format)
        {
            using namespace RHI;
            lutry
            {
                GraphicsPipelineStateDesc desc;
                InputBindingDesc bindings[] = {
                    InputBindingDesc(0, sizeof(Float2U), InputRate::per_vertex),
                    InputBindingDesc(1, sizeof(ShapeInstance), InputRate::per_instance)
                };
                InputAttributeDesc attributes[] = {
                    InputAttributeDesc(0, 0, 0, Format::rg32_float),
                    InputAttributeDesc(1, 1, offsetof(ShapeInstance, position_bounds), Format::rgba32_float),
                    InputAttributeDesc(2, 1, offsetof(ShapeInstance, shapecoord_bounds), Format::rgba32_float),
                    InputAttributeDesc(3, 1, offsetof(ShapeInstance, texcoord_bounds), Format::rgba32_float),
                    InputAttributeDesc(4, 1, offsetof(ShapeInstance, command_range_and_state), Format::rgba32_uint),
                    InputAttributeDesc(5, 1, offsetof(ShapeInstance, color), Format::rgba32_float)
                };
                desc.input_layout = InputLayoutDesc({bindings, 2}, {attributes, 6});
                desc.pipeline_layout = g_fill_playout;
                desc.vs = LUNA_CPPSL_GET_SHADER_DATA(FillVS);
                desc.ps = LUNA_CPPSL_GET_SHADER_DATA(FillPS);
                desc.blend_state = BlendDesc({ AttachmentBlendDesc(true, BlendFactor::src_alpha, BlendFactor::one_minus_src_alpha, BlendOp::add, BlendFactor::zero,
                        BlendFactor::one, BlendOp::add, ColorWriteMask::all) });
                desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, false, false, false, false, false);
                desc.depth_stencil_state = DepthStencilDesc(false, false);
                desc.num_color_attachments = 1;
                desc.color_formats[0] = rt_format;
                luset(m_fill_pso, get_main_device()->new_graphics_pipeline_state(desc));
            }
            lucatchret;
            return ok;
        }
        RV FillShapeRenderer::begin(RHI::ITexture* render_target)
        {
            lutsassert();
            if(render_target)
            {
                auto desc = render_target->get_desc();
                lutry
                {
                    if (m_rt_format != desc.format)
                    {
                        luexp(create_pso(desc.format));
                        m_rt_format = desc.format;
                    }
                    m_render_target = render_target;
                    m_screen_width = desc.width;
                    m_screen_height = desc.height;
                }
                lucatchret;
            }
            else 
            {
                m_render_target = nullptr;
                m_screen_width = 0;
                m_screen_height = 0;
            }
            m_draw_commands.clear();
            m_draw_calls.clear();
            return ok;
        }
        void FillShapeRenderer::draw(
            RHI::IBuffer* instance_buffer,
            RHI::IBuffer* state_buffer,
            Span<const ShapeDrawCall> draw_calls,
            Float4x4U* transform_matrix
        )
        {
            if(draw_calls.size() == 0)
            {
                return;
            }
            // Create draw command.
            m_draw_commands.emplace_back();
            DrawCommand& cmd = m_draw_commands.back();
            cmd.instance_buffer = instance_buffer;
            cmd.state_buffer = state_buffer;
            cmd.num_draw_calls = draw_calls.size();
            if (transform_matrix)
            {
                cmd.transform_matrix = *transform_matrix;
            }
            else
            {
                Float4x4 mat = ProjectionMatrix::make_orthographic_off_center(0.0f, (f32)m_screen_width, 0.0f, (f32)m_screen_height, 0.0f, 1.0f);
                cmd.transform_matrix = mat;
            }
            // Create draw calls.
            m_draw_calls.insert(m_draw_calls.end(), draw_calls);
        }
        struct CBData
        {
            Float4x4U transform;
        };
        RV FillShapeRenderer::end()
        {
            using namespace RHI;
            lutsassert();
            lucheck_msg(m_render_target, "Call IShapeRenderer::begin() firstly before render()!");
            auto dev = get_main_device();
            if(m_draw_calls.size() == 0)
            {
                return ok;
            }
            lutry
            {
                // Creates one new render call.
                u32 cb_element_size = (u32)align_upper(sizeof(CBData), dev->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment);
                usize num_draw_calls = m_draw_calls.size();
                u64 cb_size = cb_element_size * num_draw_calls;
                // Build constant buffer.
                if (num_draw_calls > m_cbs_capacity)
                {
                    luset(m_cbs_resource, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer,
                        cb_size)));
                    m_cbs_capacity = num_draw_calls;
                }
                void* cb_data = nullptr;
                luexp(m_cbs_resource->map(0, 0, &cb_data));
                usize dc_index = 0;
                for(usize i = 0; i < m_draw_commands.size(); ++i)
                {
                    auto& cmd = m_draw_commands[i];
                    for(usize j = 0; j < cmd.num_draw_calls; ++j)
                    {
                        CBData* dst = (CBData*)(((usize)cb_data) + dc_index * cb_element_size);
                        dst->transform = cmd.transform_matrix;
                        ++dc_index;
                    }
                }
                m_cbs_resource->unmap(0, cb_size);
                // Build view sets.
                usize command_index = 0;
                usize command_end = m_draw_commands[0].num_draw_calls;
                for (usize i = 0; i < num_draw_calls; ++i)
                {
                    while (i >= command_end)
                    {
                        ++command_index;
                        command_end += m_draw_commands[command_index].num_draw_calls;
                    }
                    while (m_desc_sets.size() <= i)
                    {
                        lulet(desc_set, dev->new_descriptor_set(DescriptorSetDesc(g_fill_desc_layout)));
                        m_desc_sets.push_back(desc_set);
                    }
                    auto& ds = m_desc_sets[i];
                    auto& dc = m_draw_calls[i];
                    auto& cmd = m_draw_commands[command_index];
                    auto num_points = dc.shape_buffer->get_desc().size / sizeof(f32);
                    auto num_states = cmd.state_buffer->get_desc().size / sizeof(ShapeState);
                    luexp(ds->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_cbs_resource, i * cb_element_size)),
                        WriteDescriptorSet::read_buffer_view(1, BufferViewDesc::structured_buffer(dc.shape_buffer, 0, num_points, 4)),
                        WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(dc.texture ? dc.texture : g_white_tex)),
                        WriteDescriptorSet::sampler(3, dc.sampler),
                        WriteDescriptorSet::read_buffer_view(4, BufferViewDesc::structured_buffer(cmd.state_buffer, 0,
                            num_states, sizeof(ShapeState)))
                        }));
                }
            }
            lucatchret;
            return ok;
        }
        void FillShapeRenderer::prepare(RHI::ICommandBuffer* cmdbuf)
        {
            using namespace RHI;
            usize num_draw_calls = m_draw_calls.size();
            Vector<BufferBarrier> buffer_barriers;
            for (const DrawCommand& cmd : m_draw_commands)
            {
                buffer_barriers.push_back(BufferBarrier(cmd.state_buffer,
                    BufferStateFlag::automatic, BufferStateFlag::shader_read_vs));
            }
            Vector<TextureBarrier> barriers;
            barriers.push_back({ g_white_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none });
            for (usize i = 0; i < num_draw_calls; ++i)
            {
                if (m_draw_calls[i].texture)
                {
                    barriers.push_back({ m_draw_calls[i].texture, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none });
                }
            }
            cmdbuf->resource_barrier({buffer_barriers.data(), (u32)buffer_barriers.size()},
                {barriers.data(), (u32)barriers.size()});
        }
        void FillShapeRenderer::submit(RHI::ICommandBuffer* cmdbuf, u32 first_draw_call, u32 num_draw_calls)
        {
            using namespace RHI;
            usize total_draw_calls = m_draw_calls.size();
            usize range_begin = min<usize>(first_draw_call, total_draw_calls);
            usize range_end = num_draw_calls == U32_MAX ? total_draw_calls :
                min<usize>(range_begin + num_draw_calls, total_draw_calls);
            if(range_begin == range_end)
            {
                return;
            }
            cmdbuf->set_graphics_pipeline_state(m_fill_pso);
            cmdbuf->set_graphics_pipeline_layout(g_fill_playout);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)m_screen_width, (f32)m_screen_height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, m_screen_width, m_screen_height));
            usize dc_index = 0;
            for(usize i = 0; i < m_draw_commands.size(); ++i)
            {
                auto& cmd = m_draw_commands[i];
                usize command_begin = dc_index;
                usize command_end = command_begin + cmd.num_draw_calls;
                if(command_end <= range_begin || command_begin >= range_end)
                {
                    dc_index = command_end;
                    continue;
                }
                auto num_instances = cmd.instance_buffer->get_desc().size / sizeof(ShapeInstance);
                VertexBufferView vertex_views[] = {
                    VertexBufferView(g_quad_vertex_buffer, 0, sizeof(Float2U) * 4, sizeof(Float2U)),
                    VertexBufferView(cmd.instance_buffer, 0,
                        sizeof(ShapeInstance) * num_instances, sizeof(ShapeInstance))
                };
                cmdbuf->set_vertex_buffers(0, {vertex_views, 2});
                cmdbuf->set_index_buffer(IndexBufferView(g_quad_index_buffer, 0,
                    sizeof(u16) * 6, Format::r16_uint));
                for (usize j = 0; j < cmd.num_draw_calls; ++j)
                {
                    if(dc_index >= range_begin && dc_index < range_end)
                    {
                        IDescriptorSet* ds = m_desc_sets[dc_index];
                        cmdbuf->set_graphics_descriptor_sets(0, { &ds, 1 });
                        cmdbuf->draw_indexed_instanced(6, m_draw_calls[dc_index].num_instances,
                            0, 0, m_draw_calls[dc_index].base_instance);
                    }
                    ++dc_index;
                }
            }
        }
        LUNA_VG_API Ref<IShapeRenderer> new_fill_shape_renderer()
        {
            Ref<IShapeRenderer> ret;
            Ref<FillShapeRenderer> renderer = new_object<FillShapeRenderer>();
            ret = renderer;
            return ret;
        }
    }
}
