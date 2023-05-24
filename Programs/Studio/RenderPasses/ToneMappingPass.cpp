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
#include "../StudioHeader.hpp"

namespace Luna
{
    RV ToneMappingPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
			// Histogram Clear Pass.
			{
				luset(m_histogram_clear_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::read_write_buffer_view, 0, 1, ShaderVisibilityFlag::compute)
					})));
				auto dlayout = m_histogram_clear_pass_dlayout.get();
				luset(m_histogram_clear_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ &dlayout, 1 },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("LumHistogramClear.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.cs = cs_blob.cspan();
				ps_desc.shader_input_layout = m_histogram_clear_pass_slayout;
				luset(m_histogram_clear_pass_pso, device->new_compute_pipeline_state(ps_desc));
			}
			// Histogram Lum Pass.
			{
				luset(m_histogram_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 1, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_write_buffer_view, 2, 1, ShaderVisibilityFlag::compute)
					})));
				auto dlayout = m_histogram_pass_dlayout.get();
				luset(m_histogram_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ &dlayout, 1 },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
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
			// Histogram Collect Pass.
			{
				luset(m_histogram_collect_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_write_buffer_view, 1, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_write_texture_view, 2, 1, ShaderVisibilityFlag::compute)
					})));
				auto dlayout = m_histogram_collect_pass_dlayout.get();
				luset(m_histogram_collect_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ &dlayout, 1 },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
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
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 1, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 2, 1, ShaderVisibilityFlag::compute),
					DescriptorSetLayoutBinding(DescriptorType::read_write_texture_view, 3, 1, ShaderVisibilityFlag::compute)
					})));
				auto dlayout = m_tone_mapping_pass_dlayout.get();
				luset(m_tone_mapping_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ &dlayout, 1 },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
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
			luset(m_histogram_clear_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_histogram_clear_pass_dlayout)));
            luset(m_histogram_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_histogram_pass_dlayout)));
            luset(m_histogram_collect_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_histogram_collect_pass_dlayout)));
            luset(m_tone_mapping_pass_ds, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_tone_mapping_pass_dlayout)));
            auto cb_align = device->get_uniform_buffer_data_alignment();
            luset(m_histogram_cb, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(LumHistogramParams), cb_align))));
			luset(m_histogram_collect_cb, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(LumHistogramCollectParams), cb_align))));
			luset(m_tone_mapping_cb, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(ToneMappingParams), cb_align))));
			luset(m_lum_tex, device->new_texture(MemoryType::local, TextureDesc::tex2d(Format::r32_float,
				TextureUsageFlag::read_write_texture | TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1)));
			f32 value = 0.0f;
			luexp(upload_texture_data(m_lum_tex, SubresourceIndex(0, 0), 0, 0, 0, &value, 4, 4, 1, 1, 1));
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
			Ref<ITexture> lighting_tex = ctx->get_input("hdr_texture");
			Ref<ITexture> output_tex = ctx->get_output("ldr_texture");
			auto lighting_tex_desc = lighting_tex->get_desc();
            auto output_tex_desc = output_tex->get_desc();
            auto cb_align = cmdbuf->get_device()->get_uniform_buffer_data_alignment();
			constexpr f32 min_brightness = 0.001f;
			constexpr f32 max_brightness = 20.0f;
            // Tone mapping pass.
			{
				cmdbuf->set_context(CommandBufferContextType::compute);
				Ref<IBuffer> m_histogram_buffer;
				luset(m_histogram_buffer, ctx->allocate_temporary_resource(RG::ResourceDesc::as_buffer(MemoryType::local, BufferDesc(BufferUsageFlag::read_write_buffer, sizeof(u32) * 256))));
				cmdbuf->attach_device_object(m_histogram_buffer);
				// Histogram Clear Pass.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_histogram_clear_pass_slayout);
					cmdbuf->set_compute_pipeline_state(m_global_data->m_histogram_clear_pass_pso);
					cmdbuf->resource_barrier({
							BufferBarrier(m_histogram_buffer, BufferStateFlag::automatic, BufferStateFlag::shader_write_cs)
						}, {});
					auto vs = m_histogram_clear_ds.get();
					vs->update_descriptors({
						WriteDescriptorSet::read_write_buffer_view(0, BufferViewDesc::typed_buffer(m_histogram_buffer, 0, 256, Format::r32_uint))
						});
					cmdbuf->set_compute_descriptor_sets(0, { &vs, 1 });
					cmdbuf->dispatch(1, 1, 1);
				}
				// Histogram Lum Pass.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_histogram_pass_slayout);
					cmdbuf->set_compute_pipeline_state(m_global_data->m_histogram_pass_pso);
					lulet(mapped, m_histogram_cb->map(0, 0));
					((LumHistogramParams*)mapped)->src_width = lighting_tex_desc.width;
					((LumHistogramParams*)mapped)->src_height = lighting_tex_desc.height;
					((LumHistogramParams*)mapped)->min_brightness = min_brightness;
					((LumHistogramParams*)mapped)->max_brightness = max_brightness;
					m_histogram_cb->unmap(0, sizeof(LumHistogramParams));
					cmdbuf->resource_barrier({
							BufferBarrier(m_histogram_buffer, BufferStateFlag::shader_write_cs, BufferStateFlag::shader_read_cs | BufferStateFlag::shader_write_cs),
							BufferBarrier(m_histogram_cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs),
						}, {
							TextureBarrier(lighting_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs)
						});
					auto vs = m_histogram_ds.get();
					vs->update_descriptors({
						WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_histogram_cb, 0, (u32)align_upper(sizeof(LumHistogramParams), cb_align))),
						WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(lighting_tex)),
						WriteDescriptorSet::read_write_buffer_view(2, BufferViewDesc::typed_buffer(m_histogram_buffer, 0, 256, Format::r32_uint))
						});
					cmdbuf->set_compute_descriptor_sets(0, { &vs, 1 });
					cmdbuf->dispatch(align_upper(lighting_tex_desc.width, 16) / 16, 
						align_upper(lighting_tex_desc.height, 16) / 16, 1);
				}

				// Histogram Collect Lum passes.
				{
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_histogram_collect_pass_slayout);
					cmdbuf->set_compute_pipeline_state(m_global_data->m_histogram_collect_pass_pso);
					lulet(mapped, m_histogram_collect_cb->map(0, 0));
					((LumHistogramCollectParams*)mapped)->min_brightness = min_brightness;
					((LumHistogramCollectParams*)mapped)->max_brightness = max_brightness;
					((LumHistogramCollectParams*)mapped)->time_coeff = 0.05f;
					((LumHistogramCollectParams*)mapped)->num_pixels = (u32)lighting_tex_desc.width * lighting_tex_desc.height;
					m_histogram_collect_cb->unmap(0, sizeof(LumHistogramCollectParams));
					cmdbuf->resource_barrier({
							BufferBarrier(m_histogram_collect_cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs),
							BufferBarrier(m_histogram_buffer, BufferStateFlag::shader_write_cs, BufferStateFlag::shader_read_cs | BufferStateFlag::shader_write_cs)
						}, {
							TextureBarrier(m_lum_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs | TextureStateFlag::shader_write_cs)
						});
					auto vs = m_histogram_collect_ds.get();
					vs->update_descriptors({
						WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_histogram_collect_cb, 0, (u32)align_upper(sizeof(LumHistogramCollectParams), cb_align))),
						WriteDescriptorSet::read_write_buffer_view(1, BufferViewDesc::typed_buffer(m_histogram_buffer, 0, 256, Format::r32_uint)),
						WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(m_lum_tex))
						});
					cmdbuf->set_compute_descriptor_sets(0, { &vs, 1 });
					cmdbuf->dispatch(1, 1, 1);
				}

				// Tone Mapping Pass.
				{
					lulet(mapped, m_tone_mapping_cb->map(0, 0));
					ToneMappingParams params;
					params.exposure = exposure;
					params.auto_exposure = auto_exposure ? 1 : 0;
					memcpy(mapped, &params, sizeof(ToneMappingParams));
					m_tone_mapping_cb->unmap(0, sizeof(ToneMappingParams));
					cmdbuf->set_compute_shader_input_layout(m_global_data->m_tone_mapping_pass_slayout);
					cmdbuf->set_compute_pipeline_state(m_global_data->m_tone_mapping_pass_pso);
					cmdbuf->resource_barrier({
						{m_tone_mapping_cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_cs, ResourceBarrierFlag::none}
						}, {
						{m_lum_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
						{lighting_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
						{output_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs | TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none}
					});
					auto vs = m_tone_mapping_pass_ds.get();
					vs->update_descriptors({
						WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(m_tone_mapping_cb, 0, (u32)align_upper(sizeof(ToneMappingParams), cb_align))),
						WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(lighting_tex)),
						WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(m_lum_tex)),
						WriteDescriptorSet::read_write_texture_view(3, TextureViewDesc::tex2d(output_tex))
						});
					cmdbuf->set_compute_descriptor_sets(0, { &vs, 1 });
					cmdbuf->dispatch((u32)align_upper(output_tex_desc.width, 8) / 8, (u32)align_upper(output_tex_desc.height, 8) / 8, 1);
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
			RG::ResourceDesc desc = compiler->get_resource_desc(hdr_texture);
			RG::ResourceDesc desc2 = compiler->get_resource_desc(ldr_texture);
			desc2.texture.width = desc.texture.width;
			desc2.texture.height = desc.texture.height;
			desc2.texture.usages |= RHI::TextureUsageFlag::read_write_texture;
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