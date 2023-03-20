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
				luset(m_histogram_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibility::all)
					})));

				luset(m_histogram_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_histogram_pass_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("LumHistogram.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.cs = cs_blob.cspan();
				ps_desc.shader_input_layout = m_histogram_pass_slayout;
				luset(m_histogram_pass_pso, device->new_compute_pipeline_state(ps_desc));
			}

			//Lum Pass.
			{
				luset(m_histogram_collect_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 1, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibility::all)
					})));
				luset(m_histogram_collect_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_histogram_collect_pass_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("LumHistogramCollect.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.cs = cs_blob.cspan();
				ps_desc.shader_input_layout = m_histogram_collect_pass_slayout;
				luset(m_histogram_collect_pass_pso, device->new_compute_pipeline_state(ps_desc));
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
		u32 auto_exposure;
	};
	struct LumHistogramParams
	{
		u32 src_width;
		u32 src_height;
		f32 min_brightness;
		f32 max_brightness;
	};
	struct LumHistogramCollectParams
	{
		f32 min_brightness;
		f32 max_brightness;
		f32 time_coeff;
		f32 num_pixels;
	};
    RV ToneMappingPass::init(ToneMappingPassGlobalData* global_data)
    {
        using namespace RHI;
        lutry
        {
            m_global_data = global_data;
            auto device = global_data->m_histogram_pass_pso->get_device();
            luset(m_histogram_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_histogram_pass_dlayout)));
            luset(m_histogram_collect_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_histogram_collect_pass_dlayout)));
            luset(m_tone_mapping_pass_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_tone_mapping_pass_dlayout)));
            auto cb_align = device->get_constant_buffer_data_alignment();
            luset(m_histogram_cb, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(LumHistogramParams), cb_align))));
			luset(m_histogram_collect_cb, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(LumHistogramCollectParams), cb_align))));
			luset(m_tone_mapping_cb, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(ToneMappingParams), cb_align))));
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
			auto lighting_tex_desc = lighting_tex->get_desc();
            auto output_tex_desc = output_tex->get_desc();
            auto cb_align = cmdbuf->get_device()->get_constant_buffer_data_alignment();
			constexpr f32 min_brightness = 0.001f;
			constexpr f32 max_brightness = 20.0f;
            // Tone mapping pass.
			{
				lulet(m_histogram_buffer, ctx->allocate_temporary_resource(ResourceDesc::buffer(ResourceHeapType::local, ResourceUsageFlag::unordered_access, sizeof(u32) * 256)));
				lulet(m_lum_tex, ctx->allocate_temporary_resource(ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::r32_float, ResourceUsageFlag::unordered_access | ResourceUsageFlag::shader_resource, 1, 1)));
				luexp(m_lum_tex->map_subresource(0, false));
				f32 v = 0.0f;
				luexp(m_lum_tex->write_subresource(0, &v, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
				m_lum_tex->unmap_subresource(0, true);
				cmdbuf->attach_device_object(m_histogram_buffer);
				cmdbuf->attach_device_object(m_lum_tex);
				// Histogram Lum Pass.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_histogram_pass_slayout);
					cmdbuf->set_pipeline_state(m_global_data->m_histogram_pass_pso);
					LumHistogramParams* mapped = nullptr;
					luexp(m_histogram_cb->map_subresource(0, false, (void**)&mapped));
					mapped->src_width = (u32)lighting_tex_desc.width_or_buffer_size;
					mapped->src_height = lighting_tex_desc.height;
					mapped->min_brightness = min_brightness;
					mapped->max_brightness = max_brightness;
					m_histogram_cb->unmap_subresource(0, true);
					cmdbuf->resource_barriers({ 
						ResourceBarrierDesc::as_transition(lighting_tex, ResourceState::shader_resource_non_pixel, 0),
						ResourceBarrierDesc::as_transition(m_histogram_buffer, ResourceState::unordered_access, 0),
						ResourceBarrierDesc::as_transition(m_histogram_cb, ResourceState::vertex_and_constant_buffer, 0) });
					auto vs = m_histogram_ds.get();
                    vs->set_cbv(0, m_histogram_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(LumHistogramParams), cb_align)));
					vs->set_srv(1, lighting_tex);
					vs->set_uav(2, m_histogram_buffer, nullptr, &UnorderedAccessViewDesc::as_buffer(Format::r32_uint, 0, 256, 0, 0, false));
					cmdbuf->set_compute_descriptor_set(0, vs);
					cmdbuf->dispatch(align_upper(lighting_tex_desc.width_or_buffer_size, 16) / 16, 
						align_upper(lighting_tex_desc.height, 16) / 16, 1);
				}

				// Histogram Collect Lum passes.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_histogram_collect_pass_slayout);
					cmdbuf->set_pipeline_state(m_global_data->m_histogram_collect_pass_pso);
					LumHistogramCollectParams* mapped = nullptr;
					luexp(m_histogram_collect_cb->map_subresource(0, false, (void**)&mapped));
					mapped->min_brightness = min_brightness;
					mapped->max_brightness = max_brightness;
					mapped->time_coeff = 0.5f;
					mapped->num_pixels = (u32)lighting_tex_desc.width_or_buffer_size * lighting_tex_desc.height;
					m_histogram_collect_cb->unmap_subresource(0, true);
					cmdbuf->resource_barriers({
							ResourceBarrierDesc::as_uav(m_histogram_buffer),
							ResourceBarrierDesc::as_transition(m_lum_tex, ResourceState::unordered_access, 0) });
					auto vs = m_histogram_collect_ds.get();
                    vs->set_cbv(0, m_histogram_collect_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(LumHistogramCollectParams), cb_align)));
					vs->set_uav(1, m_histogram_buffer, nullptr, &UnorderedAccessViewDesc::as_buffer(Format::r32_uint, 0, 256, 0, 0, false));
					vs->set_uav(2, m_lum_tex);
					cmdbuf->set_compute_descriptor_set(0, vs);
					cmdbuf->dispatch(1, 1, 1);
				}

				// Tone Mapping Pass.
				{
					void* mapped = nullptr;
					luexp(m_tone_mapping_cb->map_subresource(0, false, &mapped));
					ToneMappingParams params;
					params.exposure = exposure;
					params.auto_exposure = 1;
					memcpy(mapped, &params, sizeof(ToneMappingParams));
					m_tone_mapping_cb->unmap_subresource(0, true);
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_tone_mapping_pass_slayout);
					cmdbuf->set_pipeline_state(m_global_data->m_tone_mapping_pass_pso);
					cmdbuf->resource_barriers({
						ResourceBarrierDesc::as_transition(m_lum_tex, ResourceState::shader_resource_non_pixel),
						ResourceBarrierDesc::as_transition(lighting_tex, ResourceState::shader_resource_non_pixel),
						ResourceBarrierDesc::as_transition(output_tex, ResourceState::unordered_access),
						ResourceBarrierDesc::as_transition(m_tone_mapping_cb, ResourceState::vertex_and_constant_buffer) });
					auto vs = m_tone_mapping_pass_ds.get();
                    vs->set_cbv(0, m_tone_mapping_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(ToneMappingParams), cb_align)));
					vs->set_srv(1, lighting_tex);
					vs->set_srv(2, m_lum_tex);
					vs->set_uav(3, output_tex);
					cmdbuf->set_compute_descriptor_set(0, vs);
					cmdbuf->dispatch((u32)align_upper(output_tex_desc.width_or_buffer_size, 8) / 8, (u32)align_upper(output_tex_desc.height, 8) / 8, 1);
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
			RHI::ResourceDesc desc = compiler->get_resource_desc(hdr_texture);
			RHI::ResourceDesc desc2 = compiler->get_resource_desc(ldr_texture);
			desc2.width_or_buffer_size = desc2.width_or_buffer_size ? desc.width_or_buffer_size : desc.width_or_buffer_size;
			desc2.height = desc2.height ? desc2.height : desc.height;
			desc2.usages |= RHI::ResourceUsageFlag::unordered_access;
			compiler->set_resource_desc(ldr_texture, desc2);
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