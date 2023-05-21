/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DeferredLightingPass.cpp
* @author JXMaster
* @date 2023/3/18
*/
#include "DeferredLightingPass.hpp"
#include <Runtime/File.hpp>
#include "../SceneRenderer.hpp"
#include "../StudioHeader.hpp"

namespace Luna
{
    RV DeferredLightingPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_deferred_lighting_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 1, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 3, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 4, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 5, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 6, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 7, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::srv, 8, 1, ShaderVisibilityFlag::all),
						DescriptorSetLayoutBinding(DescriptorType::uav, 9, 1, ShaderVisibilityFlag::all),
						DescriptorSetLayoutBinding(DescriptorType::sampler, 10, 1, ShaderVisibilityFlag::all)
						})));

			luset(m_deferred_lighting_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_deferred_lighting_pass_dlayout },
				ShaderInputLayoutFlag::deny_vertex_shader_access |
				ShaderInputLayoutFlag::deny_pixel_shader_access)));

			lulet(psf, open_file("DeferredLighting.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = psf->get_size();
			auto cs_blob = Blob((usize)file_size);
			luexp(psf->read(cs_blob.span()));
			psf = nullptr;
			ComputePipelineStateDesc ps_desc;
			ps_desc.cs = cs_blob.cspan();
			ps_desc.shader_input_layout = m_deferred_lighting_pass_slayout;
			luset(m_deferred_lighting_pass_pso, device->new_compute_pipeline_state(ps_desc));

            luset(m_default_skybox, device->new_resource(ResourceDesc::tex2d(MemoryType::local, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
            u8 skybox_data[] = {0, 0, 0, 0};
            luexp(device->copy_resource({
                ResourceCopyDesc::as_write_texture(m_default_skybox, skybox_data, 4, 4, 0, BoxU(0, 0, 0, 1, 1, 1))
            }));

            // Generate integrate brdf.
            constexpr usize INTEGEATE_BRDF_SIZE = 256;
            {
                luset(m_integrate_brdf, device->new_resource(ResourceDesc::tex2d(MemoryType::local, Format::rgba8_unorm,
                    ResourceUsageFlag::shader_resource | ResourceUsageFlag::unordered_access, INTEGEATE_BRDF_SIZE, INTEGEATE_BRDF_SIZE, 1, 1)));
                lulet(dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                        DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
                        DescriptorSetLayoutBinding(DescriptorType::uav, 1, 1, ShaderVisibilityFlag::all) })));
                lulet(slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ dlayout },
                    ShaderInputLayoutFlag::deny_vertex_shader_access |
                    ShaderInputLayoutFlag::deny_domain_shader_access |
                    ShaderInputLayoutFlag::deny_geometry_shader_access |
                    ShaderInputLayoutFlag::deny_hull_shader_access |
                    ShaderInputLayoutFlag::deny_pixel_shader_access)));
                lulet(psf, open_file("PrecomputeIntegrateBRDF.cso", FileOpenFlag::read, FileCreationMode::open_existing));
                auto file_size = psf->get_size();
                auto cs_blob = Blob((usize)file_size);
                luexp(psf->read(cs_blob.span()));
                psf = nullptr;
                ComputePipelineStateDesc ps_desc;
                ps_desc.cs = cs_blob.cspan();
                ps_desc.shader_input_layout = slayout;
                lulet(pso, device->new_compute_pipeline_state(ps_desc));
                lulet(compute_cmdbuf, g_env->async_compute_queue->new_command_buffer());
                u32 cb_align = device->get_uniform_buffer_data_alignment();
                u32 cb_size = (u32)align_upper(sizeof(Float2), cb_align);
                lulet(cb, device->new_resource(
                    ResourceDesc::buffer(MemoryType::upload, BufferUsageFlag::uniform_buffer, cb_size)));
                Float2U* mapped = nullptr;
                luexp(cb->map_subresource(0, 0, 0, (void**)&mapped));
                *mapped = Float2U(1.0f / (f32)INTEGEATE_BRDF_SIZE, 1.0f / (f32)INTEGEATE_BRDF_SIZE);
                cb->unmap_subresource(0, 0, sizeof(Float2U));
                ResourceBarrierDesc barriers[] = {
                    ResourceBarrierDesc::as_transition(cb, ResourceStateFlag::vertex_and_constant_buffer),
                    ResourceBarrierDesc::as_transition(m_integrate_brdf, ResourceStateFlag::unordered_access)
                };
                compute_cmdbuf->resource_barriers({ barriers, 2 });
                lulet(vs, device->new_descriptor_set(DescriptorSetDesc(dlayout)));
                vs->set_cbv(0, cb, ConstantBufferViewDesc(0, cb_size));
                vs->set_uav(1, m_integrate_brdf);
                compute_cmdbuf->set_compute_shader_input_layout(slayout);
                compute_cmdbuf->set_pipeline_state(pso);
                compute_cmdbuf->set_compute_descriptor_set(0, vs);
                compute_cmdbuf->dispatch(align_upper(INTEGEATE_BRDF_SIZE, 8) / 8, align_upper(INTEGEATE_BRDF_SIZE, 8) / 8, 1);
                luexp(compute_cmdbuf->submit());
                compute_cmdbuf->wait();
            }
        }
        lucatchret;
        return ok;
    }
    RV DeferredLightingPass::init(DeferredLightingPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = m_global_data->m_deferred_lighting_pass_dlayout->get_device();
            luset(m_ds, device->new_descriptor_set(
                DescriptorSetDesc(global_data->m_deferred_lighting_pass_dlayout)));
            luset(m_lighting_mode_cb, device->new_resource(
                ResourceDesc::buffer(MemoryType::upload, BufferUsageFlag::uniform_buffer,
                    align_upper(sizeof(u32), device->get_uniform_buffer_data_alignment()))));
        }
        lucatchret;
        return ok;
    }
    RV DeferredLightingPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            u32* mapped;
            luexp(m_lighting_mode_cb->map_subresource(0, 0, 0, (void**)&mapped));
            *mapped = lighting_mode;
            m_lighting_mode_cb->unmap_subresource(0, 0, sizeof(u32));
            auto scene_tex = ctx->get_output("scene_texture");
            auto depth_tex = ctx->get_input("depth_texture");
            auto base_color_roughness_tex = ctx->get_input("base_color_roughness_texture");
            auto normal_metallic_tex = ctx->get_input("normal_metallic_texture");
			auto emissive_tex = ctx->get_input("emissive_texture");
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_uniform_buffer_data_alignment();
            auto sky_box = skybox ? skybox : m_global_data->m_default_skybox;
            cmdbuf->resource_barriers({
                ResourceBarrierDesc::as_transition(camera_cb, ResourceStateFlag::vertex_and_constant_buffer),
                ResourceBarrierDesc::as_transition(m_lighting_mode_cb, ResourceStateFlag::vertex_and_constant_buffer),
                ResourceBarrierDesc::as_transition(light_params, ResourceStateFlag::shader_resource_non_pixel),
				ResourceBarrierDesc::as_transition(scene_tex, ResourceStateFlag::unordered_access),
				ResourceBarrierDesc::as_transition(depth_tex, ResourceStateFlag::shader_resource_non_pixel),
				ResourceBarrierDesc::as_transition(base_color_roughness_tex, ResourceStateFlag::shader_resource_non_pixel),
                ResourceBarrierDesc::as_transition(normal_metallic_tex, ResourceStateFlag::shader_resource_non_pixel),
                ResourceBarrierDesc::as_transition(emissive_tex, ResourceStateFlag::shader_resource_non_pixel),
                ResourceBarrierDesc::as_transition(sky_box, ResourceStateFlag::shader_resource_non_pixel),
                ResourceBarrierDesc::as_transition(m_global_data->m_integrate_brdf, ResourceStateFlag::shader_resource_non_pixel) });
            
            m_ds->set_cbv(0, camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
            m_ds->set_cbv(1, m_lighting_mode_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(u32), cb_align)));
            if (light_ts.empty())
			{
				// Adds one fake light.
				m_ds->set_srv(2, light_params, &ShaderResourceViewDesc::as_buffer(Format::unknown, 0, 1, sizeof(LightingParams)));
			}
			else
			{
				m_ds->set_srv(2, light_params, &ShaderResourceViewDesc::as_buffer(Format::unknown, 0, (u32)light_ts.size(), sizeof(LightingParams)));
			}
            m_ds->set_srv(3, base_color_roughness_tex);
            m_ds->set_srv(4, normal_metallic_tex);
            m_ds->set_srv(5, emissive_tex);
            m_ds->set_srv(6, depth_tex, &ShaderResourceViewDesc::tex2d(Format::r32_float, 0, 1, 0.0f));
            m_ds->set_srv(7, sky_box);
            m_ds->set_srv(8, m_global_data->m_integrate_brdf);
            m_ds->set_uav(9, scene_tex);
            m_ds->set_sampler(10, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp));
            auto scene_desc = scene_tex->get_desc();
            cmdbuf->set_compute_shader_input_layout(m_global_data->m_deferred_lighting_pass_slayout);
            cmdbuf->set_pipeline_state(m_global_data->m_deferred_lighting_pass_pso);
            cmdbuf->set_compute_descriptor_set(0, m_ds);
            cmdbuf->dispatch((u32)align_upper(scene_desc.width_or_buffer_size, 8) / 8,
                align_upper(scene_desc.height, 8) / 8, 1);
        }
        lucatchret;
        return ok;
    }

    RV compile_deferred_lighting_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            DeferredLightingPassGlobalData* data = (DeferredLightingPassGlobalData*)userdata;
			auto scene_texture = compiler->get_output_resource("scene_texture");
			auto depth_texture = compiler->get_input_resource("depth_texture");
            auto base_color_roughness_texture = compiler->get_input_resource("base_color_roughness_texture");
            auto normal_metallic_texture = compiler->get_input_resource("normal_metallic_texture");
            auto emissive_texture = compiler->get_input_resource("emissive_texture");
			if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Output \"scene_texture\" is not specified.");
			if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"depth_texture\" is not specified.");
            if(base_color_roughness_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"base_color_roughness_texture\" is not specified.");
            if(normal_metallic_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"normal_metallic_texture\" is not specified.");
            if(emissive_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Input \"emissive_texture\" is not specified.");
			RG::ResourceDesc desc = compiler->get_resource_desc(scene_texture);
			if (desc.pixel_format != RHI::Format::rgba32_float)
			{
				return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Invalid format for \"scene_texture\" is specified. \"scene_texture\" must be Format::rgba32_float.");
			}
			desc.usages |= RHI::ResourceUsageFlag::unordered_access;
			compiler->set_resource_desc(scene_texture, desc);

			desc = compiler->get_resource_desc(depth_texture);
			if (desc.pixel_format != RHI::Format::d32_float)
			{
				return set_error(BasicError::bad_arguments(), "DeferredLightingPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be Format::d32_float.");
			}
			desc.usages |= RHI::ResourceUsageFlag::depth_stencil;
			compiler->set_resource_desc(depth_texture, desc);

            desc = compiler->get_resource_desc(base_color_roughness_texture);
			desc.usages |= RHI::ResourceUsageFlag::shader_resource;
			compiler->set_resource_desc(base_color_roughness_texture, desc);

            desc = compiler->get_resource_desc(normal_metallic_texture);
			desc.usages |= RHI::ResourceUsageFlag::shader_resource;
			compiler->set_resource_desc(normal_metallic_texture, desc);

            desc = compiler->get_resource_desc(emissive_texture);
			desc.usages |= RHI::ResourceUsageFlag::shader_resource;
			compiler->set_resource_desc(emissive_texture, desc);

			Ref<DeferredLightingPass> pass = new_object<DeferredLightingPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }

    RV register_deferred_lighting_pass()
    {
        lutry
        {
            register_boxed_type<DeferredLightingPassGlobalData>();
            register_boxed_type<DeferredLightingPass>();
            impl_interface_for_type<DeferredLightingPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "DeferredLighting";
            desc.desc = "Illuminate the scene.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture."});
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture"});
            desc.input_parameters.push_back({"base_color_roughness_texture", "The base color and roughness texture from geometry pass."});
            desc.input_parameters.push_back({"normal_metallic_texture", "The normal and metallic texture from geometry pass."});
            desc.input_parameters.push_back({"emissive_texture", "The emissive texture from geometry pass."});
            desc.compile = compile_deferred_lighting_pass;
            auto data = new_object<DeferredLightingPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}