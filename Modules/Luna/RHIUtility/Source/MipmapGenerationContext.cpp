/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MipmapGenerationContext.cpp
* @author JXMaster
* @date 2025/9/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_UTILITY_API LUNA_EXPORT
#include "MipmapGenerationContext.hpp"
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>

#include <MipmapGenerationCS.hpp>

namespace Luna
{
    namespace RHIUtility
    {
        RV MipmapGenerationPipelineState::init(RHI::IDevice* device)
        {
            using namespace RHI;
            lutry
            {
                {
                    luset(m_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                        DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::all)
                        })));
                    auto dlayout = m_dlayout.get();
                    luset(m_playout, device->new_pipeline_layout(PipelineLayoutDesc(
                        { &dlayout, 1 },
                        PipelineLayoutFlag::deny_vertex_shader_access |
                        
                        PipelineLayoutFlag::deny_pixel_shader_access)));

                    ComputePipelineStateDesc ps_desc;
                    LUNA_FILL_COMPUTE_SHADER_DATA(ps_desc, MipmapGenerationCS);
                    ps_desc.pipeline_layout = m_playout;
                    luset(m_pso, device->new_compute_pipeline_state(ps_desc));
                }
            }
            lucatchret;
            return ok;
        }

        HashMap<RHI::IDevice*, WeakRef<MipmapGenerationPipelineState>> g_mipmap_device_states;
        SpinLock g_mipmap_device_states_lock;
        void cleanup_mipmap_generation_states()
        {
            g_mipmap_device_states.clear();
            g_mipmap_device_states.shrink_to_fit();
        }

        static R<Ref<MipmapGenerationPipelineState>> get_mipmap_generation_state(RHI::IDevice* device)
        {
            LockGuard guard(g_mipmap_device_states_lock);
            Ref<MipmapGenerationPipelineState> ret;
            lutry
            {
                auto iter = g_mipmap_device_states.find(device);
                if(iter != g_mipmap_device_states.end())
                {
                    ret = iter->second.pin();
                }
                if(!ret)
                {
                    ret = new_object<MipmapGenerationPipelineState>();
                    luexp(ret->init(device));
                    g_mipmap_device_states.insert_or_assign(device, WeakRef<MipmapGenerationPipelineState>(ret));
                }
            }
            lucatchret;
            return ret;
        }

        RV MipmapGenerationContext::init(RHI::IDevice* device)
        {
            lutry
            {
                m_device = device;
                luset(m_ps, get_mipmap_generation_state(device));
            }
            lucatchret;
            return ok;
        }

        void MipmapGenerationContext::reset()
        {
            m_ops.clear();
        }

        void MipmapGenerationContext::generate_mipmaps(RHI::ITexture* tex, u32 source_mip, u32 num_gen_mips)
        {
            MipmapGenerationOp op;
            op.tex = tex;
            op.source_mip = source_mip;
            op.end_mip = min(source_mip + 1 + num_gen_mips, tex->get_desc().mip_levels);
            m_ops.push_back(move(op));
        }

        RV MipmapGenerationContext::commit(RHI::ICommandBuffer* compute_cmdbuf, bool submit_and_wait)
        {
            using namespace RHI;
            lutry
            {
                u32 cb_align = (u32)m_device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
                u32 cb_entry_size = (u32)align_upper(sizeof(Float2), cb_align);
                u32 num_ops = 0;
                for(auto& op : m_ops)
                {
                    num_ops += op.end_mip - (op.source_mip + 1);
                }
                if(num_ops == 0) return ok;
                u32 cb_size = cb_entry_size * num_ops;
                if(!m_cb || m_cb->get_desc().size < cb_size)
                {
                    luset(m_cb, m_device->new_buffer(MemoryType::upload,
                        BufferDesc(BufferUsageFlag::uniform_buffer, cb_entry_size * num_ops)));
                }
                void* mapped = nullptr;
                luexp(m_cb->map(0, 0, &mapped));
                u32 op_i = 0;
                for(auto& op : m_ops)
                {
                    auto desc = op.tex->get_desc();
                    for (u32 mip = op.source_mip + 1; mip < op.end_mip; ++mip)
                    {
                        u32 width = max<u32>((u32)desc.width >> mip, 1);
                        u32 height = max<u32>(desc.height >> mip, 1);
                        Float2U* dst = (Float2U*)((usize)mapped + cb_entry_size * op_i);
                        dst->x = 1.0f / (f32)width;
                        dst->y = 1.0f / (f32)height;
                        ++op_i;
                    }
                }
                m_cb->unmap(0, cb_size);
                while(m_dss.size() < num_ops)
                {
                    lulet(vs, m_device->new_descriptor_set(DescriptorSetDesc(m_ps->m_dlayout)));
                    m_dss.push_back(move(vs));
                }
                op_i = 0;
                compute_cmdbuf->begin_compute_pass();
                compute_cmdbuf->set_compute_pipeline_layout(m_ps->m_playout);
                compute_cmdbuf->set_compute_pipeline_state(m_ps->m_pso);
                for(auto& op : m_ops)
                {
                    auto desc = op.tex->get_desc();
                    for(u32 mip = op.source_mip + 1; mip < op.end_mip; ++mip)
                    {
                        TextureBarrier barriers[] = {
                            {op.tex, SubresourceIndex(mip - 1, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
                            {op.tex, SubresourceIndex(mip, 0),TextureStateFlag::automatic, TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none}
                        };
                        compute_cmdbuf->resource_barrier({}, { barriers, 2 });
                        luexp(m_dss[op_i]->update_descriptors({
                            WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_cb, cb_entry_size * op_i, cb_entry_size)),
                            WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(op.tex, Format::unknown, mip - 1, 1)),
                            WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(op.tex, Format::unknown, mip, 1)),
                            WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                        }));
                        compute_cmdbuf->set_compute_descriptor_set(0, m_dss[op_i]);
                        u32 width = desc.width >> mip;
                        u32 height = desc.height >> mip;
                        compute_cmdbuf->dispatch((u32)align_upper(width, 8) / 8, (u32)align_upper(height, 8) / 8, 1);
                        ++op_i;
                    }
                }
                compute_cmdbuf->end_compute_pass();
                if(submit_and_wait)
                {
                    luexp(compute_cmdbuf->submit({}, {}, true));
                    compute_cmdbuf->wait();
                    luexp(compute_cmdbuf->reset());
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_RHI_UTILITY_API R<Ref<IMipmapGenerationContext>> new_mipmap_generation_context(RHI::IDevice* device)
        {
            Ref<IMipmapGenerationContext> r;
            lutry
            {
                auto ctx = new_object<MipmapGenerationContext>();
                luexp(ctx->init(device));
                r = ctx;
            }
            lucatchret;
            return r;
        }
    }
}