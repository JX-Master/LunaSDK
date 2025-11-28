/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BlitContext.cpp
* @author JXMaster
* @date 2025/11/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "BlitContext.hpp"
#include <BlitVS.hpp>
#include <BlitPS.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        struct BlitVertex
        {
            Float2U position;
            Float2U texcoord;
        };
        RV BlitContext::init(RHI::IDevice* device, RHI::Format dst_format)
        {
            using namespace RHI;
            lutry
            {
                m_device = device;
                luset(m_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 0, 1, ShaderVisibilityFlag::pixel),
                    DescriptorSetLayoutBinding::sampler(1, 1, ShaderVisibilityFlag::pixel)
                    })));
                auto dlayout = m_dlayout.get();
                luset(m_playout, device->new_pipeline_layout(PipelineLayoutDesc(
                    { &dlayout, 1 },
                    PipelineLayoutFlag::allow_input_assembler_input_layout)));
                GraphicsPipelineStateDesc ps_desc;
                ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
                ps_desc.blend_state = BlendDesc();
                ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::none, 0, 0.0f, 0.0f, false, true);
                ps_desc.depth_stencil_state = DepthStencilDesc(false, false, CompareFunction::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
                ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
                InputBindingDesc input_bindings[] = {
                    InputBindingDesc(0, sizeof(BlitVertex), InputRate::per_vertex)
                };
                InputAttributeDesc input_attributes[] = {
                    InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rg32_float),
                    InputAttributeDesc("TEXCOORD", 0, 1, 0, 8, Format::rg32_float)
                };
                ps_desc.input_layout.bindings = { input_bindings, 1 };
                ps_desc.input_layout.attributes = { input_attributes , 2 };
                ps_desc.vs = LUNA_GET_SHADER_DATA(BlitVS);
                ps_desc.ps = LUNA_GET_SHADER_DATA(BlitPS);
                ps_desc.pipeline_layout = m_playout.get();
                ps_desc.num_color_attachments = 1;
                ps_desc.color_formats[0] = dst_format;
                luset(m_pso, device->new_graphics_pipeline_state(ps_desc));
                luset(m_ib, device->new_buffer(MemoryType::upload, RHI::BufferDesc(BufferUsageFlag::index_buffer, sizeof(u16) * 6)));
                void* mapped;
                luexp(m_ib->map(0, 0, &mapped));
                constexpr u16 ib_data[] = {0, 1, 2, 1, 3, 2};
                memcpy(mapped, ib_data, sizeof(ib_data));
                m_ib->unmap(0, sizeof(u16) * 6);
            }
            lucatchret;
            return ok;
        }
        void BlitContext::blit(RHI::ITexture* dst, const RHI::SubresourceIndex& dst_subresource,
                const RHI::TextureViewDesc& src,
                const RHI::SamplerDesc& sampler,
                const Float2U& top_left, const Float2U& top_right,
                const Float2U& bottom_left, const Float2U& bottom_right)
        {
            BlitDrawCall draw_call;
            draw_call.dst = dst;
            draw_call.src = src;
            draw_call.dst_subresource = dst_subresource;
            draw_call.src = src;
            draw_call.sampler = sampler;
            draw_call.top_left = top_left;
            draw_call.top_right = top_right;
            draw_call.bottom_left = bottom_left;
            draw_call.bottom_right = bottom_right;
            m_draw_calls.push_back(draw_call);
        }
        inline Float2U pos_to_ndc(const Float2& pos, u32 width, u32 height)
        {
            Float2 ret = pos;
            ret /= Float2(width, height);
            ret = ret * 2.0f - 1.0f;
            return Float2U(ret.x, -ret.y);
        }
        RV BlitContext::commit(RHI::ICommandBuffer* graphics_cmdbuf, bool submit_and_wait)
        {
            lutry
            {
                // resize buffers.
                usize vb_size = m_draw_calls.size() * sizeof(BlitVertex) * 4;
                usize ib_size = m_draw_calls.size() * sizeof(u32) * 6;
                RHI::IDevice* device = m_device;
                if(vb_size > m_vb_capacity)
                {
                    luset(m_vb, device->new_buffer(RHI::MemoryType::upload, RHI::BufferDesc(RHI::BufferUsageFlag::vertex_buffer, vb_size)));
                    m_vb_capacity = vb_size;
                }
                while(m_desc_sets.size() < m_draw_calls.size())
                {
                    lulet(desc_set, device->new_descriptor_set(RHI::DescriptorSetDesc(m_dlayout)));
                    m_desc_sets.push_back(desc_set);
                }
                // upload data.
                BlitVertex* vertices;
                luexp(m_vb->map(0, 0, (void**)&vertices));
                u32 dc_index = 0;
                for(auto& dc : m_draw_calls)
                {
                    RHI::ITexture* dst = dc.dst;
                    auto desc = dst->get_desc();
                    u32 width = desc.width;
                    u32 height = desc.height;
                    vertices[0] = {pos_to_ndc(dc.top_left, width, height), Float2U(0, 0)};
                    vertices[1] = {pos_to_ndc(dc.top_right, width, height), Float2U(1, 0)};
                    vertices[2] = {pos_to_ndc(dc.bottom_left, width, height), Float2U(0, 1)};
                    vertices[3] = {pos_to_ndc(dc.bottom_right, width, height), Float2U(1, 1)};
                    vertices += 4;
                    luexp(m_desc_sets[dc_index]->update_descriptors({
                        RHI::WriteDescriptorSet::read_texture_view(0, dc.src),
                        RHI::WriteDescriptorSet::sampler(1, dc.sampler)
                    }));
                    ++dc_index;
                }
                m_vb->unmap(0, vb_size);

                // Emit draw calls.
                graphics_cmdbuf->resource_barrier({
                    RHI::BufferBarrier(m_vb, RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::vertex_buffer),
                    RHI::BufferBarrier(m_ib, RHI::BufferStateFlag::automatic, RHI::BufferStateFlag::index_buffer)
                }, {});
                Vector<RHI::TextureBarrier> barriers;
                graphics_cmdbuf->begin_event("Blit Context");
                dc_index = 0;
                for(auto& dc : m_draw_calls)
                {
                    barriers.clear();
                    barriers.push_back(RHI::TextureBarrier(dc.dst, dc.dst_subresource, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::color_attachment_write));
                    u32 src_num_mips = dc.src.texture->get_desc().mip_levels;
                    u32 src_num_sampled_mips = min(src_num_mips - dc.src.mip_slice, dc.src.mip_size);
                    lucheck_msg(dc.src.type == RHI::TextureViewType::tex2d, "BlitContext: only tex2d src texture view is supported.");
                    for(u32 i = 0; i < src_num_sampled_mips; ++i)
                    {
                        barriers.push_back(RHI::TextureBarrier(dc.src.texture, RHI::SubresourceIndex(dc.src.mip_slice + i, dc.src.array_slice), RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::shader_read_ps));
                    }
                    graphics_cmdbuf->resource_barrier({}, {barriers.cspan()});
                    RHI::RenderPassDesc render_pass;
                    render_pass.color_attachments[0] = RHI::ColorAttachment(dc.dst, RHI::LoadOp::load, RHI::StoreOp::store, Float4U(0), RHI::TextureViewType::tex2d, RHI::Format::unknown, dc.dst_subresource.mip_slice, dc.dst_subresource.array_slice);
                    graphics_cmdbuf->begin_render_pass(render_pass);
                    graphics_cmdbuf->set_graphics_pipeline_layout(m_playout);
                    graphics_cmdbuf->set_graphics_pipeline_state(m_pso);
                    graphics_cmdbuf->set_graphics_descriptor_set(0, m_desc_sets[dc_index]);
                    graphics_cmdbuf->set_vertex_buffers(0, {RHI::VertexBufferView(m_vb, dc_index * sizeof(BlitVertex) * 4, sizeof(BlitVertex) * 4, sizeof(BlitVertex))});
                    graphics_cmdbuf->set_index_buffer(RHI::IndexBufferView(m_ib, 0, sizeof(u16) * 6, RHI::Format::r16_uint));
                    auto dst_desc = dc.dst->get_desc();
                    graphics_cmdbuf->set_viewport(RHI::Viewport(0, 0, dst_desc.width, dst_desc.height, 0, 1));
                    graphics_cmdbuf->set_scissor_rect(RectI(0, 0, dst_desc.width, dst_desc.height));
                    graphics_cmdbuf->draw_indexed(6, 0, 0);
                    graphics_cmdbuf->end_render_pass();
                    ++dc_index;
                }
                graphics_cmdbuf->end_event();
                if(submit_and_wait)
                {
                    luexp(graphics_cmdbuf->submit({}, {}, true));
                    graphics_cmdbuf->wait();
                    luexp(graphics_cmdbuf->reset());
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_RHI_UTILITY_API R<Ref<IBlitContext>> new_blit_context(RHI::IDevice* device, RHI::Format dst_format)
        {
            Ref<IBlitContext> ret;
            lutry
            {
                auto obj = new_object<BlitContext>();
                luexp(obj->init(device, dst_format));
                ret = obj;
            }
            lucatchret;
            return ret;
        }
    }
}
