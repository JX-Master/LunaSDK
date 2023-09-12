
/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SkyBoxPass.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include "SkyBoxPass.hpp"

#include <Luna/Runtime/File.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>
#include "../StudioHeader.hpp"
namespace Luna
{
    RV SkyBoxPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_skybox_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::compute),
						DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::compute),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 2, 1, ShaderVisibilityFlag::compute),
						DescriptorSetLayoutBinding::read_write_texture_view(TextureViewType::tex2d, 3, 1, ShaderVisibilityFlag::compute),
						DescriptorSetLayoutBinding::sampler(4, 1, ShaderVisibilityFlag::compute)
						})));
            auto dl = m_skybox_pass_dlayout.get();
			luset(m_skybox_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dl, 1 },
				PipelineLayoutFlag::deny_vertex_shader_access |
				PipelineLayoutFlag::deny_pixel_shader_access)));
            lulet(cs_blob, compile_shader("Shaders/SkyboxCS.hlsl", ShaderCompiler::ShaderType::compute));
			ComputePipelineStateDesc ps_desc;
			ps_desc.cs = cs_blob.cspan();
			ps_desc.pipeline_layout = m_skybox_pass_playout;
			luset(m_skybox_pass_pso, device->new_compute_pipeline_state(ps_desc));
        }
        lucatchret;
        return ok;
    }
    struct SkyboxParams
	{
		Float4x4U view_to_world;
		f32 fov;
		u32 width;
		u32 height;
	};
    RV SkyBoxPass::init(SkyBoxPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = m_global_data->m_skybox_pass_pso->get_device();
            auto cb_align = device->get_uniform_buffer_data_alignment();
            luset(m_skybox_params_cb, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(SkyboxParams), cb_align))));
            luset(m_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_skybox_pass_dlayout)));
        }
        lucatchret;
        return ok;
    }
    RV SkyBoxPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto cmdbuf = ctx->get_command_buffer();
            Ref<ITexture> output_tex = ctx->get_output(m_global_data->m_texture_name);
            Ref<ITexture> depth_tex = ctx->get_input(m_global_data->m_depth_texture_name);
            u32 time_query_begin, time_query_end;
            auto query_heap = ctx->get_timestamp_query_heap(&time_query_begin, &time_query_end);
			if (skybox && camera_type == CameraType::perspective)
			{
				// Draw skybox.
                SkyboxParams* mapped = nullptr;
				luexp(m_skybox_params_cb->map(0, 0, (void**)&mapped));
				auto camera_forward_dir = mul(Float4(0.0f, 0.0f, 1.0f, 0.0f), view_to_world);
				memcpy(&mapped->view_to_world, &view_to_world, sizeof(Float4x4));
                mapped->fov = camera_fov;
                auto desc = output_tex->get_desc();
                mapped->width = (u32)desc.width;
                mapped->height = (u32)desc.height;
				m_skybox_params_cb->unmap(0, sizeof(SkyboxParams));
                ComputePassDesc compute_pass;
                if(query_heap)
                {
                    compute_pass.timestamp_query_heap = query_heap;
                    compute_pass.timestamp_query_begin_pass_write_index = time_query_begin;
                    compute_pass.timestamp_query_end_pass_write_index = time_query_end;
                }
                cmdbuf->begin_compute_pass(compute_pass);
				cmdbuf->resource_barrier(
                    {
                        BufferBarrier(m_skybox_params_cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs)
                    },
                    {
                        TextureBarrier(output_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_write_cs),
                        TextureBarrier(skybox, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                        TextureBarrier(depth_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs),
                    });
				cmdbuf->set_compute_pipeline_layout(m_global_data->m_skybox_pass_playout);
				cmdbuf->set_compute_pipeline_state(m_global_data->m_skybox_pass_pso);
                auto cb_align = cmdbuf->get_device()->get_uniform_buffer_data_alignment();
                luexp(m_ds->update_descriptors({
                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_skybox_params_cb, 0, (u32)align_upper(sizeof(SkyboxParams), cb_align))),
                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(skybox)),
                    WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(depth_tex)),
                    WriteDescriptorSet::read_write_texture_view(3, TextureViewDesc::tex2d(output_tex)),
                    WriteDescriptorSet::sampler(4, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat))
                    }));
                auto ds = m_ds.get();
                cmdbuf->set_compute_descriptor_sets(0, {&ds, 1});
				cmdbuf->dispatch(align_upper((u32)desc.width, 8) / 8, (u32)align_upper(desc.height, 8) / 8, 1);
                cmdbuf->end_compute_pass();
			}
			else
			{
				// Clears to black.
                cmdbuf->resource_barrier({}, {
                    TextureBarrier(output_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write)
                    });
				auto lighting_rt = output_tex;
				RenderPassDesc render_pass;
				render_pass.color_attachments[0] = ColorAttachment(output_tex, LoadOp::clear, StoreOp::store, Float4U(0.0f));
                if(query_heap)
                {
                    render_pass.timestamp_query_heap = query_heap;
                    render_pass.timestamp_query_begin_pass_write_index = time_query_begin;
                    render_pass.timestamp_query_end_pass_write_index = time_query_end;
                }
				cmdbuf->begin_render_pass(render_pass);
				cmdbuf->end_render_pass();
			}
        }
        lucatchret;
        return ok;
    }
    RV compile_sky_box_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            SkyBoxPassGlobalData* data = (SkyBoxPassGlobalData*)userdata;
            auto texture_resource = compiler->get_output_resource(data->m_texture_name);
            auto depth_texture_resource = compiler->get_input_resource(data->m_depth_texture_name);
            if(texture_resource == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "SkyBoxPass: Output \"texture\" is not specified.");
            if(depth_texture_resource == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "SkyBoxPass: Input \"depth_texture\" is not specified.");
            
            RG::ResourceDesc depth_desc = compiler->get_resource_desc(depth_texture_resource);
            if (depth_desc.type != RG::ResourceType::texture || depth_desc.texture.type != RHI::TextureType::tex2d ||
                !depth_desc.texture.width || !depth_desc.texture.height)
            {
                return set_error(BasicError::bad_arguments(), "SkyBoxPass: The resource format for input \"depth_texture\" is not specified or invalid.");
            }
            depth_desc.texture.usages |= RHI::TextureUsageFlag::read_texture;
            compiler->set_resource_desc(depth_texture_resource, depth_desc);

            RG::ResourceDesc desc = compiler->get_resource_desc(texture_resource);
            desc.texture.width = depth_desc.texture.width;
            desc.texture.height = depth_desc.texture.height;
            desc.texture.usages |= RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::color_attachment;
            compiler->set_resource_desc(texture_resource, desc);
            Ref<SkyBoxPass> pass = new_object<SkyBoxPass>();
            luexp(pass->init(data));
            compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_sky_box_pass()
    {
        lutry
        {
            register_boxed_type<SkyBoxPassGlobalData>();
            register_boxed_type<SkyBoxPass>();
            impl_interface_for_type<SkyBoxPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "SkyBox";
            desc.desc = "Renders one sky box to the specified background using the specified texture";
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture."});
            desc.output_parameters.push_back({"texture", "The render target to render the sky box to."});
            desc.compile = compile_sky_box_pass;
            auto data = new_object<SkyBoxPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}
