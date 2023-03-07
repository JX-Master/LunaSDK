/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ToneMappingPass.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include "ToneMappingPass.hpp"
#include <Runtime/File.hpp>

namespace Luna
{
    RV ToneMappingPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            //First Lum Pass.
			{
				luset(m_first_lum_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibility::all)
					})));

				luset(m_first_lum_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_first_lum_pass_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("LumFirstCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.cs = cs_blob.cspan();
				ps_desc.shader_input_layout = m_first_lum_pass_slayout;
				luset(m_first_lum_pass_pso, device->new_compute_pipeline_state(ps_desc));
			}

			//Lum Pass.
			{
				luset(m_lum_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::srv, 0, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 1, 1, ShaderVisibility::all)
					})));
				luset(m_lum_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_lum_pass_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("LumCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.cs = cs_blob.cspan();
				ps_desc.shader_input_layout = m_lum_pass_slayout;
				luset(m_lum_pass_pso, device->new_compute_pipeline_state(ps_desc));
			}

			//Tone Mapping Pass.
			{
				luset(m_tone_mapping_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 3, 1, ShaderVisibility::all)
					})));
				luset(m_tone_mapping_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_tone_mapping_pass_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("ToneMappingCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.cs = cs_blob.cspan();
				ps_desc.shader_input_layout = m_tone_mapping_pass_slayout;
				luset(m_tone_mapping_pass_pso, device->new_compute_pipeline_state(ps_desc));
			}
        }
        lucatchret;
        return ok;
    }
	struct ToneMappingParams
	{
		f32 exposure = 1.0f;
	};
    RV ToneMappingPass::init(ToneMappingPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = global_data->m_lum_pass_pso->get_device();
            luset(m_first_lum_pass_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_first_lum_pass_dlayout)));
            for(usize i = 0; i < 10; ++i)
            {
                luset(m_lum_pass_dss[i], device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_lum_pass_dlayout)));
            }
            luset(m_tone_mapping_pass_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_tone_mapping_pass_dlayout)));
            auto cb_align = device->get_constant_buffer_data_alignment();
            luset(m_tone_mapping_offset, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(Float4) * 16, cb_align))));
			luset(m_tone_mapping_params, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(ToneMappingParams), cb_align))));
            u32 sz = 1024;
			for (u32 i = 0; i < 11; ++i)
			{
				luset(m_lighting_accms[i], device->new_resource(ResourceDesc::tex2d(
					ResourceHeapType::local,
					Format::r32_float,
					ResourceUsageFlag::unordered_access | ResourceUsageFlag::shader_resource,
					sz, sz, 1, 1)));

				sz >>= 1;
			}
        }
        lucatchret;
        return ok;
    }
    RV ToneMappingPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto cmdbuf = ctx->get_command_buffer();
            auto lighting_tex = ctx->get_input("hdr_texture");
            auto output_tex = ctx->get_output("ldr_texture");
            auto output_tex_desc = output_tex->get_desc();
            auto cb_align = cmdbuf->get_device()->get_constant_buffer_data_alignment();
            // Tone mapping pass.
			{
				// First Lum Pass.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_first_lum_pass_slayout);
					cmdbuf->set_pipeline_state(m_global_data->m_first_lum_pass_pso);
					Float4 offsets[16];
					{
						// How much texels are covered by one sample pixel?
						f32 TexelsCoveredPerSampleW = output_tex_desc.width_or_buffer_size / 1024.0f;
						f32 TexelsCoveredPerSampleH = output_tex_desc.height / 1024.0f;
						// The offset of one texel in uv-space.
						f32 NormalizedWidthPerTexel = 1.0f / output_tex_desc.width_or_buffer_size;
						f32 NormalizedHeightPerTexel = 1.0f / output_tex_desc.height;
						for (i32 i = 0; i < 4; i++)
						{
							for (i32 j = 0; j < 4; j++)
							{
								offsets[4 * i + j] = Float4{
									NormalizedWidthPerTexel * TexelsCoveredPerSampleW / 8.0f * (float)(2 * j - 3) ,
									NormalizedHeightPerTexel * TexelsCoveredPerSampleH / 8.0f * (float)(2 * i - 3) ,0.0f,0.0f };
							}
						}
					}
					void* mapped = nullptr;
					luexp(m_tone_mapping_offset->map_subresource(0, false, &mapped));
					memcpy(mapped, offsets, sizeof(Float4) * 16);
					m_tone_mapping_offset->unmap_subresource(0, true);
					cmdbuf->resource_barriers({ 
						ResourceBarrierDesc::as_transition(lighting_tex, ResourceState::shader_resource_non_pixel, 0),
						ResourceBarrierDesc::as_transition(m_lighting_accms[0], ResourceState::unordered_access, 0),
						ResourceBarrierDesc::as_transition(m_tone_mapping_offset, ResourceState::vertex_and_constant_buffer, 0) });
					auto vs = m_first_lum_pass_ds.get();
                    vs->set_cbv(0, m_tone_mapping_offset, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(Float4) * 16, cb_align)));
					vs->set_srv(1, lighting_tex);
					vs->set_uav(2, m_lighting_accms[0]);
					vs->set_sampler(3, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
					cmdbuf->set_compute_descriptor_set(0, vs);
					cmdbuf->dispatch(128, 128, 1);
				}

				// Lum passes.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_lum_pass_slayout);
					cmdbuf->set_pipeline_state(m_global_data->m_lum_pass_pso);
					for (u32 i = 0; i < 10; ++i)
					{
						cmdbuf->resource_barriers({
							ResourceBarrierDesc::as_transition(m_lighting_accms[i], ResourceState::shader_resource_non_pixel, 0),
							ResourceBarrierDesc::as_transition(m_lighting_accms[i + 1], ResourceState::unordered_access, 0) });
						auto vs = m_lum_pass_dss[i].get();
                        vs->set_srv(0, m_lighting_accms[i]);
						vs->set_uav(1, m_lighting_accms[i + 1]);
						cmdbuf->set_compute_descriptor_set(0, vs);
						u32 dispatches = max(64 >> i, 1);
						cmdbuf->dispatch(dispatches, dispatches, 1);
					}
				}

				// Tone Mapping Pass.
				{
					void* mapped = nullptr;
					luexp(m_tone_mapping_params->map_subresource(0, false, &mapped));
					ToneMappingParams params;
					params.exposure = exposure;
					memcpy(mapped, &params, sizeof(ToneMappingParams));
					m_tone_mapping_params->unmap_subresource(0, true);
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_tone_mapping_pass_slayout);
					cmdbuf->set_pipeline_state(m_global_data->m_tone_mapping_pass_pso);
					cmdbuf->resource_barriers({
						ResourceBarrierDesc::as_transition(m_lighting_accms[10], ResourceState::shader_resource_non_pixel),
						ResourceBarrierDesc::as_transition(lighting_tex, ResourceState::shader_resource_non_pixel),
						ResourceBarrierDesc::as_transition(output_tex, ResourceState::unordered_access),
						ResourceBarrierDesc::as_transition(m_tone_mapping_params, ResourceState::vertex_and_constant_buffer) });
					auto vs = m_tone_mapping_pass_ds.get();
                    vs->set_cbv(0, m_tone_mapping_params, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(ToneMappingParams), cb_align)));
					vs->set_srv(1, lighting_tex);
					vs->set_srv(2, m_lighting_accms[10]);
					vs->set_uav(3, output_tex);
					cmdbuf->set_compute_descriptor_set(0, vs);
					cmdbuf->dispatch(max<u32>((u32)output_tex_desc.width_or_buffer_size / 8, 1), max<u32>((u32)output_tex_desc.height / 8, 1), 1);
				}
			}
        }
        lucatchret;
        return ok;
    }

    RV compile_tone_mapping_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            ToneMappingPassGlobalData* data = (ToneMappingPassGlobalData*)userdata;
			auto hdr_texture = compiler->get_input_resource("hdr_texture");
			auto ldr_texture = compiler->get_output_resource("ldr_texture");

			if(hdr_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "ToneMappingPass: Input \"hdr_texture\" is not specified.");
			if(ldr_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "ToneMappingPass: Output \"ldr_texture\" is not specified.");

			// Set output texture format if not specified.
			RHI::ResourceDesc desc;
			if(compiler->get_resource_desc(hdr_texture, &desc))
			{
				RHI::ResourceDesc desc2;
				if(!compiler->get_resource_desc(ldr_texture, &desc2))
				{
					desc2 = RHI::ResourceDesc::tex2d(RHI::ResourceHeapType::local, RHI::Format::rgba8_unorm, RHI::ResourceUsageFlag::shader_resource, desc.width_or_buffer_size, desc.height);
				}
				desc2.usages |= RHI::ResourceUsageFlag::unordered_access;
				compiler->set_resource_desc(ldr_texture, desc2);
			}
			else
			{
				set_error(BasicError::bad_arguments(), "ToneMappingPass: The resource layout for input \"hdr_texture\" is not specified.");
			}
            Ref<ToneMappingPass> pass = new_object<ToneMappingPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_tone_mapping_pass()
    {
        lutry
        {
            register_boxed_type<ToneMappingPassGlobalData>();
            register_boxed_type<ToneMappingPass>();
            impl_interface_for_type<ToneMappingPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "ToneMapping";
            desc.desc = "Converts HDR image to LDR image.";
            desc.input_parameters.push_back({"hdr_texture", "The HDR image."});
            desc.output_parameters.push_back({"ldr_texture", "The result image"});
            desc.compile = compile_tone_mapping_pass;
            auto data = new_object<ToneMappingPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}