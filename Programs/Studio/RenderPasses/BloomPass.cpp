/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file BloomPass.cpp
* @author JXMaster
* @date 2024/4/24
*/
#include "BloomPass.hpp"
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <BloomSetupCS.hpp>
#include <BloomDownSampleCS.hpp>
#include <BloomUpSampleCS.hpp>

namespace Luna
{
    RV BloomPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            {
                ComputePipelineStateDesc desc;
                LUNA_FILL_COMPUTE_SHADER_DATA(desc, BloomSetupCS);
                luset(m_bloom_setup_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::compute)
                })));
                luset(m_bloom_setup_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({m_bloom_setup_pass_dlayout}, PipelineLayoutFlag::deny_vertex_shader_access | PipelineLayoutFlag::deny_pixel_shader_access)));
                desc.pipeline_layout = m_bloom_setup_pass_playout;
                luset(m_bloom_setup_pass_pso, device->new_compute_pipeline_state(desc));
            }
            {
                ComputePipelineStateDesc desc;
                LUNA_FILL_COMPUTE_SHADER_DATA(desc, BloomDownSampleCS);
                luset(m_bloom_downsample_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::sampler(3, 1, ShaderVisibilityFlag::compute)
                })));
                luset(m_bloom_downsample_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({m_bloom_downsample_pass_dlayout}, PipelineLayoutFlag::deny_vertex_shader_access | PipelineLayoutFlag::deny_pixel_shader_access)));
                desc.pipeline_layout = m_bloom_downsample_pass_playout;
                luset(m_bloom_downsample_pass_pso, device->new_compute_pipeline_state(desc));
            }
            {
                ComputePipelineStateDesc desc;
                LUNA_FILL_COMPUTE_SHADER_DATA(desc, BloomUpSampleCS);
                luset(m_bloom_upsample_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                    DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 3, 1, ShaderVisibilityFlag::compute),
                    DescriptorSetLayoutBinding::sampler(4, 1, ShaderVisibilityFlag::compute)
                })));
                luset(m_bloom_upsample_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({m_bloom_upsample_pass_dlayout}, PipelineLayoutFlag::deny_vertex_shader_access | PipelineLayoutFlag::deny_pixel_shader_access)));
                desc.pipeline_layout = m_bloom_upsample_pass_playout;
                luset(m_bloom_upsample_pass_pso, device->new_compute_pipeline_state(desc));
            }
        }
        lucatchret;
        return ok;
    }
    struct BloomSetupParams
    {
        u32 dst_tex_width;
        u32 dst_tex_height;
        f32 lum_threshold;
    };
    struct BloomDownSampleParams
    {
        u32 dst_tex_width;
        u32 dst_tex_height;
    };
    struct BloomUpSampleParams
    {
        u32 src_tex_width;
        u32 src_tex_height;
        u32 dst_tex_width;
        u32 dst_tex_height;
        f32 up_sample_radius;
    };
    RV BloomPass::init(BloomPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = m_global_data->m_bloom_downsample_pass_pso->get_device();
            luset(m_setup_pass.m_ds, device->new_descriptor_set(DescriptorSetDesc(global_data->m_bloom_setup_pass_dlayout)));
            luset(m_setup_pass.m_params, device->new_buffer(MemoryType::upload, 
                BufferDesc(BufferUsageFlag::uniform_buffer,
                    align_upper(sizeof(BloomSetupParams), 
                    device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment))));
        }
        lucatchret;
        return ok;
    }
    RV BloomPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            Ref<ITexture> src_tex = ctx->get_input("scene_texture");
            Ref<ITexture> dst_tex = ctx->get_output("bloom_texture");

            auto cmdbuf = ctx->get_command_buffer();

            // Allocate downsample / upsample chain.
            auto desc = src_tex->get_desc();

            u32 num_downsample_mips = 1;
            u32 downsample_width = desc.width >> 1;
            u32 downsample_height = desc.height >> 1;
            while(downsample_width > 10 && downsample_height > 10)
            {
                ++num_downsample_mips;
                downsample_width >>= 1;
                downsample_height >>= 1;
            }
            if(num_downsample_mips <= 1)
            {
                // screen size is too small to bloom.
                cmdbuf->resource_barrier({}, {
                    TextureBarrier(dst_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::color_attachment_write)
                });
                RenderPassDesc desc;
                desc.color_attachments[0] = ColorAttachment(dst_tex, LoadOp::clear, StoreOp::store);
                cmdbuf->begin_render_pass(desc);
                cmdbuf->end_render_pass();
                return ok;
            }
            u32 width = desc.width >> 1;
            u32 height = desc.height >> 1;
            TextureDesc ds_desc = TextureDesc::tex2d(desc.format, TextureUsageFlag::read_texture | TextureUsageFlag::read_write_texture, 
                width, height, 1, num_downsample_mips);
            TextureDesc us_desc = TextureDesc::tex2d(desc.format, TextureUsageFlag::read_texture | TextureUsageFlag::read_write_texture | TextureUsageFlag::copy_source,
                width, height, 1, num_downsample_mips - 1);
            Ref<ITexture> ds_mips;
            Ref<ITexture> us_mips;
            luset(ds_mips, ctx->allocate_temporary_resource(RG::ResourceDesc::as_texture(MemoryType::local, ds_desc)));
            luset(us_mips, ctx->allocate_temporary_resource(RG::ResourceDesc::as_texture(MemoryType::local, us_desc)));
            
            // Setup pass data.
            auto device = m_global_data->m_bloom_downsample_pass_pso->get_device();
            auto ub_alignment = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            while(m_downsample_passes.size() < num_downsample_mips - 1)
            {
                SamplePassData data;
                luset(data.m_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_bloom_downsample_pass_dlayout)));
                luset(data.m_params, device->new_buffer(MemoryType::upload, 
                    BufferDesc(BufferUsageFlag::uniform_buffer,
                        align_upper(sizeof(BloomDownSampleParams), ub_alignment))));
                m_downsample_passes.push_back(move(data));
            }
            while(m_upsample_passes.size() < num_downsample_mips - 1)
            {
                SamplePassData data;
                luset(data.m_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_bloom_upsample_pass_dlayout)));
                luset(data.m_params, device->new_buffer(MemoryType::upload, 
                    BufferDesc(BufferUsageFlag::uniform_buffer,
                        align_upper(sizeof(BloomUpSampleParams), ub_alignment))));
                m_upsample_passes.push_back(move(data));
            }
            {
                BloomSetupParams* params = nullptr;
                luexp(m_setup_pass.m_params->map(0, 0, (void**)&params));
                params->dst_tex_width = width;
                params->dst_tex_height = height;
                params->lum_threshold = lum_threshold;
                m_setup_pass.m_params->unmap(0, sizeof(BloomSetupParams));
                luexp(m_setup_pass.m_ds->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_setup_pass.m_params, 0, align_upper(sizeof(BloomSetupParams), ub_alignment))),
                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(src_tex, Format::unknown, 0, 1)),
                    WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(ds_mips, Format::unknown, 0, 1)),
                    WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                }));
            }
            for(u32 i = 0; i < num_downsample_mips - 1; ++i)
            {
                BloomDownSampleParams* params = nullptr;
                auto& pass = m_downsample_passes[i];
                luexp(pass.m_params->map(0, 0, (void**)&params));
                params->dst_tex_width = width >> (i + 1);
                params->dst_tex_height = height >> (i + 1);
                pass.m_params->unmap(0, sizeof(BloomDownSampleParams));
                luexp(pass.m_ds->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(pass.m_params, 0, align_upper(sizeof(BloomDownSampleParams), ub_alignment))),
                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(ds_mips, Format::unknown, i, 1)),
                    WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(ds_mips, Format::unknown, i + 1, 1)),
                    WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                }));
            }
            for(u32 i = 0; i < num_downsample_mips - 1; ++i)
            {
                BloomUpSampleParams* params = nullptr;
                auto& pass = m_upsample_passes[i];
                luexp(pass.m_params->map(0, 0, (void**)&params));
                params->src_tex_width = width >> (num_downsample_mips - 1 - i);
                params->src_tex_height = height >> (num_downsample_mips - 1 - i);
                params->dst_tex_width = width >> (num_downsample_mips - 2 - i);
                params->dst_tex_height = height >> (num_downsample_mips - 2 - i);
                params->up_sample_radius = up_sample_radius;
                pass.m_params->unmap(0, sizeof(BloomUpSampleParams));
                if(i == 0)
                {
                    luexp(pass.m_ds->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(pass.m_params, 0, align_upper(sizeof(BloomUpSampleParams), ub_alignment))),
                        WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(ds_mips, Format::unknown, num_downsample_mips - 1, 1)),
                        WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(ds_mips, Format::unknown, num_downsample_mips - 2, 1)),
                        WriteDescriptorSet::read_write_texture_view(3, TextureViewDesc::tex2d(us_mips, Format::unknown, num_downsample_mips - 2, 1)),
                        WriteDescriptorSet::sampler(4, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                    }));
                }
                else
                {
                    luexp(pass.m_ds->update_descriptors({
                        WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(pass.m_params, 0, align_upper(sizeof(BloomUpSampleParams), ub_alignment))),
                        WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(us_mips, Format::unknown, num_downsample_mips - 1 - i, 1)),
                        WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(ds_mips, Format::unknown, num_downsample_mips - 2 - i, 1)),
                        WriteDescriptorSet::read_write_texture_view(3, TextureViewDesc::tex2d(us_mips, Format::unknown, num_downsample_mips - 2 - i, 1)),
                        WriteDescriptorSet::sampler(4, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                    }));
                }
            }
            // Bloom setup pass.
            {
                Vector<BufferBarrier> buffer_barriers;
                buffer_barriers.reserve(1 + m_downsample_passes.size() + m_upsample_passes.size());
                buffer_barriers.push_back(BufferBarrier(m_setup_pass.m_params, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs));
                for(auto& p : m_downsample_passes)
                {
                    buffer_barriers.push_back(BufferBarrier(p.m_params, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs));
                }
                for(auto& p : m_upsample_passes)
                {
                    buffer_barriers.push_back(BufferBarrier(p.m_params, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs));
                }
                cmdbuf->resource_barrier(buffer_barriers.cspan(), {
                    TextureBarrier(src_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                    TextureBarrier(ds_mips, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_write_cs),
                    TextureBarrier(dst_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_dest)
                });
                ComputePassDesc compute_pass;
                u32 time_query_begin, time_query_end;
                auto query_heap = ctx->get_timestamp_query_heap(&time_query_begin, &time_query_end);
                if(query_heap)
                {
                    compute_pass.timestamp_query_heap = query_heap;
                    compute_pass.timestamp_query_begin_pass_write_index = time_query_begin;
                    compute_pass.timestamp_query_end_pass_write_index = time_query_end;
                }
                cmdbuf->begin_compute_pass(compute_pass);
                cmdbuf->set_compute_pipeline_layout(m_global_data->m_bloom_setup_pass_playout);
                cmdbuf->set_compute_pipeline_state(m_global_data->m_bloom_setup_pass_pso);
                cmdbuf->set_compute_descriptor_set(0, m_setup_pass.m_ds);
                cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
            }
            // Bloom down sample pass.
            {
                cmdbuf->set_compute_pipeline_layout(m_global_data->m_bloom_downsample_pass_playout);
                cmdbuf->set_compute_pipeline_state(m_global_data->m_bloom_downsample_pass_pso);
                for(u32 i = 0; i < num_downsample_mips - 1; ++i)
                {
                    cmdbuf->resource_barrier({}, {
                        TextureBarrier(ds_mips, SubresourceIndex(i, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                        TextureBarrier(ds_mips, SubresourceIndex(i + 1, 0), TextureStateFlag::automatic, TextureStateFlag::shader_write_cs),
                    });
                    cmdbuf->set_compute_descriptor_set(0, m_downsample_passes[i].m_ds);
                    cmdbuf->dispatch(align_upper(width >> (i + 1), 8) / 8, align_upper(height >> (i + 1), 8) / 8, 1);
                }
            }
            // Bloom up sample pass.
            {
                cmdbuf->set_compute_pipeline_layout(m_global_data->m_bloom_upsample_pass_playout);
                cmdbuf->set_compute_pipeline_state(m_global_data->m_bloom_upsample_pass_pso);
                for(u32 i = 0; i < num_downsample_mips - 1; ++i)
                {
                    if(i == 0)
                    {
                        cmdbuf->resource_barrier({}, {
                            TextureBarrier(ds_mips, SubresourceIndex(num_downsample_mips - 1, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                            TextureBarrier(ds_mips, SubresourceIndex(num_downsample_mips - 2, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                            TextureBarrier(us_mips, SubresourceIndex(num_downsample_mips - 2, 0), TextureStateFlag::automatic, TextureStateFlag::shader_write_cs)
                        });
                    }
                    else
                    {
                        cmdbuf->resource_barrier({}, {
                            TextureBarrier(us_mips, SubresourceIndex(num_downsample_mips - 1 - i, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                            TextureBarrier(ds_mips, SubresourceIndex(num_downsample_mips - 2 - i, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                            TextureBarrier(us_mips, SubresourceIndex(num_downsample_mips - 2 - i, 0), TextureStateFlag::automatic, TextureStateFlag::shader_write_cs)
                        });
                    }
                    cmdbuf->set_compute_descriptor_set(0, m_upsample_passes[i].m_ds);
                    cmdbuf->dispatch(align_upper(width >> (num_downsample_mips - 2 - i), 8) / 8, align_upper(height >> (num_downsample_mips - 2 - i), 8) / 8, 1);
                }
            }
            // Copy result.
            {
                cmdbuf->end_compute_pass();
                cmdbuf->resource_barrier({}, {
                    TextureBarrier(us_mips, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_source),
                    TextureBarrier(dst_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_dest)
                });
                cmdbuf->begin_copy_pass();
                cmdbuf->copy_texture(dst_tex, SubresourceIndex(0, 0), 0, 0, 0, us_mips, SubresourceIndex(0, 0), 0, 0, 0, width, height, 1);
                cmdbuf->end_copy_pass();
            }
        }
        lucatchret;
        return ok;
    }
    RV compile_bloom_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            BloomPassGlobalData* data = (BloomPassGlobalData*)userdata;
            auto src_texture = compiler->get_input_resource("scene_texture");
            auto dst_texture = compiler->get_output_resource("bloom_texture");
            if(src_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "BloomPass: Input \"scene_texture\" is not specified.");
            if(dst_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "BloomPass: Output \"bloom_texture\" is not specified.");
            RG::ResourceDesc src_desc = compiler->get_resource_desc(src_texture);
            luassert(src_desc.type == RG::ResourceType::texture);
            src_desc.texture.usages |= RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture;
            compiler->set_resource_desc(src_texture, src_desc);
            RG::ResourceDesc dst_desc = compiler->get_resource_desc(dst_texture);
            luassert(dst_desc.type == RG::ResourceType::texture);
            dst_desc.texture.usages |= RHI::TextureUsageFlag::copy_dest;
            dst_desc.texture.format = src_desc.texture.format;
            dst_desc.texture.width = src_desc.texture.width >> 1;
            dst_desc.texture.height = src_desc.texture.height >> 1;
            dst_desc.texture.mip_levels = 1;
            compiler->set_resource_desc(dst_texture, dst_desc);
            Ref<BloomPass> pass = new_object<BloomPass>();
            luexp(pass->init(data));
            compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_bloom_pass()
    {
        lutry
        {
            register_boxed_type<BloomPassGlobalData>();
            register_boxed_type<BloomPass>();
            impl_interface_for_type<BloomPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Bloom";
            desc.desc = "Add bloom effects to the scene";
            desc.input_parameters.push_back({"scene_texture", "The scene texture."});
            desc.output_parameters.push_back({"bloom_texture", "The bloom result texture."});
            desc.compile = compile_bloom_pass;
            auto data = new_object<BloomPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}