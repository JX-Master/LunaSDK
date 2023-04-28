
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

#include <Runtime/File.hpp>
#include <Asset/Asset.hpp>
#include <Runtime/Math/Matrix.hpp>
namespace Luna
{
    RV SkyBoxPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_skybox_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::uav, 3, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::sampler, 4, 1, ShaderVisibility::all)
						})));

			luset(m_skybox_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_skybox_pass_dlayout },
				ShaderInputLayoutFlag::deny_vertex_shader_access |
				ShaderInputLayoutFlag::deny_domain_shader_access |
				ShaderInputLayoutFlag::deny_geometry_shader_access |
				ShaderInputLayoutFlag::deny_hull_shader_access |
				ShaderInputLayoutFlag::deny_pixel_shader_access)));

			lulet(psf, open_file("SkyboxCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = psf->get_size();
			auto cs_blob = Blob((usize)file_size);
			luexp(psf->read(cs_blob.span()));
			psf = nullptr;
			ComputePipelineStateDesc ps_desc;
			ps_desc.cs = cs_blob.cspan();
			ps_desc.shader_input_layout = m_skybox_pass_slayout;
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
            auto cb_align = device->get_constant_buffer_data_alignment();
            luset(m_skybox_params_cb, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(SkyboxParams), cb_align))));
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
            auto output_tex = ctx->get_output(m_global_data->m_texture_name);
            auto depth_tex = ctx->get_input(m_global_data->m_depth_texture_name);
			if (skybox && camera_type == CameraType::perspective)
			{
				// Draw skybox.
				SkyboxParams* mapped = nullptr;
				luexp(m_skybox_params_cb->map_subresource(0, 0, 0, (void**)&mapped));
				auto camera_forward_dir = mul(Float4(0.0f, 0.0f, 1.0f, 0.0f), view_to_world);
				memcpy(&mapped->view_to_world, &view_to_world, sizeof(Float4x4));
				mapped->fov = camera_fov;
                auto desc = output_tex->get_desc();
				mapped->width = (u32)desc.width_or_buffer_size;
				mapped->height = (u32)desc.height;
				m_skybox_params_cb->unmap_subresource(0, 0, sizeof(SkyboxParams));
				cmdbuf->resource_barriers({
					ResourceBarrierDesc::as_transition(output_tex, ResourceState::unordered_access),
					ResourceBarrierDesc::as_transition(skybox, ResourceState::shader_resource_non_pixel),
                    ResourceBarrierDesc::as_transition(depth_tex, ResourceState::shader_resource_non_pixel),
					ResourceBarrierDesc::as_transition(m_skybox_params_cb, ResourceState::vertex_and_constant_buffer)
					});
				cmdbuf->set_compute_shader_input_layout(m_global_data->m_skybox_pass_slayout);
				cmdbuf->set_pipeline_state(m_global_data->m_skybox_pass_pso);
                auto cb_align = cmdbuf->get_device()->get_constant_buffer_data_alignment();
				m_ds->set_cbv(0, m_skybox_params_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(SkyboxParams), cb_align)));
				m_ds->set_srv(1, skybox);
                m_ds->set_srv(2, depth_tex, &ShaderResourceViewDesc::as_tex2d(Format::r32_float, 0, 1, 0.0f));
				m_ds->set_uav(3, output_tex);
				m_ds->set_sampler(4, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
				cmdbuf->set_compute_descriptor_set(0, m_ds);
				cmdbuf->dispatch(align_upper((u32)desc.width_or_buffer_size, 8) / 8, (u32)align_upper(desc.height, 8) / 8, 1);
			}
			else
			{
				// Clears to black.
				cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(output_tex, ResourceState::render_target));
				auto lighting_rt = output_tex;
				RenderPassDesc render_pass;
                lulet(output_tex_rtv, cmdbuf->get_device()->new_render_target_view(output_tex));
				render_pass.color_attachments[0] = output_tex_rtv;
				render_pass.color_load_ops[0] = LoadOp::clear;
				render_pass.color_store_ops[0] = StoreOp::store;
				render_pass.color_clear_values[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
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
            
            RHI::ResourceDesc depth_desc = compiler->get_resource_desc(depth_texture_resource);
            if (depth_desc.type != RHI::ResourceType::texture_2d ||
                !depth_desc.width_or_buffer_size || !depth_desc.height)
            {
                return set_error(BasicError::bad_arguments(), "SkyBoxPass: The resource format for input \"depth_texture\" is not specified or invalid.");
            }
            depth_desc.usages |= RHI::ResourceUsageFlag::shader_resource;
            compiler->set_resource_desc(depth_texture_resource, depth_desc);

            RHI::ResourceDesc desc = compiler->get_resource_desc(texture_resource);
            desc.width_or_buffer_size = desc.width_or_buffer_size ? desc.width_or_buffer_size : depth_desc.width_or_buffer_size;
            desc.height = desc.height ? desc.height : depth_desc.height;
            desc.usages |= RHI::ResourceUsageFlag::unordered_access | RHI::ResourceUsageFlag::render_target;
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