/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeRenderer.cpp
* @author JXMaster
* @date 2022/4/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "ShapeRenderer.hpp"
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/RHI/Utility.hpp>
#include <FillVS.hpp>
#include <FillPS.hpp>

namespace Luna
{
    namespace VG
    {
        Ref<RHI::IDescriptorSetLayout> g_fill_desc_layout;
        Ref<RHI::IPipelineLayout> g_fill_playout;
        Ref<RHI::ITexture> g_white_tex;

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
                        DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::pixel)
                    };
                    DescriptorSetLayoutDesc desc({bindings, 4});
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
                        luexp(copy_resource_data(upload_cmdbuf, {
                            CopyResourceData::write_texture(g_white_tex, SubresourceIndex(0, 0), 0, 0, 0, 
                            &data, sizeof(data), sizeof(data), 1, 1, 1)
                        }));
                    }
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
        }
        RV FillShapeRenderer::create_pso(RHI::Format rt_format)
        {
            using namespace RHI;
            lutry
            {
                GraphicsPipelineStateDesc desc;
                InputBindingDesc bindings[] = {
                    InputBindingDesc(0, sizeof(Vertex), InputRate::per_vertex)
                };
                InputAttributeDesc attributes[] = {
                    InputAttributeDesc("POSITION", 0, 0, 0, offsetof(Vertex, position), Format::rg32_float),
                    InputAttributeDesc("SHAPECOORD", 0, 1, 0, offsetof(Vertex, shapecoord), Format::rg32_float),
                    InputAttributeDesc("TEXCOORD", 0, 2, 0, offsetof(Vertex, texcoord), Format::rg32_float),
                    InputAttributeDesc("COMMAND_OFFSET", 0, 3, 0, offsetof(Vertex, begin_command), Format::r32_uint),
                    InputAttributeDesc("NUM_COMMANDS", 0, 4, 0, offsetof(Vertex, num_commands), Format::r32_uint),
                    InputAttributeDesc("COLOR", 0, 5, 0, offsetof(Vertex, color), Format::rgba32_float)
                };
                desc.input_layout = InputLayoutDesc({bindings, 1}, {attributes, 6});
                desc.pipeline_layout = g_fill_playout;
                desc.vs = LUNA_GET_SHADER_DATA(FillVS);
                desc.ps = LUNA_GET_SHADER_DATA(FillPS);
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
            RHI::IBuffer* vertex_buffer,
            RHI::IBuffer* index_buffer,
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
            cmd.vertex_buffer = vertex_buffer;
            cmd.index_buffer = index_buffer;
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
            Float4U clip_rect;
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
                        Float4x4 transform = m_draw_calls[dc_index].transform;
                        transform = mul(transform, cmd.transform_matrix);
                        dst->transform = transform;
                        auto& clip_rect = m_draw_calls[dc_index].clip_rect;
                        dst->clip_rect = Float4U{clip_rect.offset_x, clip_rect.offset_y, clip_rect.width, clip_rect.height};
                        ++dc_index;
                    }
                }
                m_cbs_resource->unmap(0, cb_size);
                // Build view sets.
                for (usize i = 0; i < num_draw_calls; ++i)
                {
                    while (m_desc_sets.size() <= i)
                    {
                        lulet(desc_set, dev->new_descriptor_set(DescriptorSetDesc(g_fill_desc_layout)));
                        m_desc_sets.push_back(desc_set);
                    }
                    auto& ds = m_desc_sets[i];
                    auto& dc = m_draw_calls[i];
                    auto num_points = dc.shape_buffer->get_desc().size / sizeof(f32);
                    luexp(ds->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_cbs_resource, i * cb_element_size)),
                        WriteDescriptorSet::read_buffer_view(1, BufferViewDesc::structured_buffer(dc.shape_buffer, 0, num_points, 4)),
                        WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(dc.texture ? dc.texture : g_white_tex)),
                        WriteDescriptorSet::sampler(3, dc.sampler)
                        }));
                }
            }
            lucatchret;
            return ok;
        }
        void FillShapeRenderer::submit(RHI::ICommandBuffer* cmdbuf)
        {
            // Build command buffer.
            using namespace RHI;
            usize num_draw_calls = m_draw_calls.size();
            Vector<TextureBarrier> barriers;
            barriers.push_back({ m_render_target, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content });
            barriers.push_back({ g_white_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none });
            for (usize i = 0; i < num_draw_calls; ++i)
            {
                if (m_draw_calls[i].texture)
                {
                    barriers.push_back({ m_draw_calls[i].texture, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none });
                }
            }
            cmdbuf->resource_barrier({}, { barriers.data(), (u32)barriers.size()});
            RenderPassDesc desc;
            desc.color_attachments[0] = ColorAttachment(m_render_target, LoadOp::load, StoreOp::store);
            cmdbuf->begin_render_pass(desc);
            cmdbuf->set_graphics_pipeline_state(m_fill_pso);
            cmdbuf->set_graphics_pipeline_layout(g_fill_playout);
            cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)m_screen_width, (f32)m_screen_height, 0.0f, 1.0f));
            cmdbuf->set_scissor_rect(RectI(0, 0, m_screen_width, m_screen_height));
            usize dc_index = 0;
            for(usize i = 0; i < m_draw_commands.size(); ++i)
            {
                auto& cmd = m_draw_commands[i];
                auto num_vertices = cmd.vertex_buffer->get_desc().size / sizeof(Vertex);
                auto view = VertexBufferView(cmd.vertex_buffer, 0, sizeof(Vertex) * num_vertices, sizeof(Vertex));
                cmdbuf->set_vertex_buffers(0, { &view, 1 });
                auto num_indices = cmd.index_buffer->get_desc().size / sizeof(u32);
                cmdbuf->set_index_buffer({cmd.index_buffer, 0, (u32)num_indices * (u32)sizeof(u32), Format::r32_uint});
                for (usize j = 0; j < cmd.num_draw_calls; ++j)
                {
                    IDescriptorSet* ds = m_desc_sets[dc_index];
                    cmdbuf->set_graphics_descriptor_sets(0, { &ds, 1 });
                    cmdbuf->draw_indexed(m_draw_calls[dc_index].num_indices, m_draw_calls[dc_index].base_index, 0);
                    ++dc_index;
                }
            }
            cmdbuf->end_render_pass();
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
