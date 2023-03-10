/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file LightingPass.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include "LightingPass.hpp"
#include <Runtime/File.hpp>
#include <ShaderCompiler/ShaderCompiler.hpp>
#include <RHI/ShaderCompileHelper.hpp>
#include <Asset/Asset.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include "../Material.hpp"
#include "../Assets/SceneEditor.hpp"

namespace Luna
{
    RV LightingPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_lighting_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
				DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
				DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 3, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 4, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 5, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 6, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 7, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 8, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::sampler, 9, 1, ShaderVisibility::pixel)
				})));

			luset(m_lighting_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_lighting_pass_dlayout },
				ShaderInputLayoutFlag::allow_input_assembler_input_layout |
				ShaderInputLayoutFlag::deny_domain_shader_access |
				ShaderInputLayoutFlag::deny_geometry_shader_access |
				ShaderInputLayoutFlag::deny_hull_shader_access)));

			lulet(psf, open_file("LightingPassPixel.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = psf->get_size();
			auto ps_blob = Blob((usize)file_size);
			luexp(psf->read(ps_blob.span()));
			psf = nullptr;

			m_common_vertex = new_object<CommonVertex>();
			luexp(m_common_vertex->init());

			GraphicPipelineStateDesc ps_desc;
			ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
			ps_desc.sample_mask = U32_MAX;
			ps_desc.sample_quality = 0;
			ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(false, false, BlendFactor::src_alpha,
				BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
			ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
			ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
			ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			ps_desc.input_layout = m_common_vertex->input_layout_common;
			ps_desc.vs = m_common_vertex->vs_blob.cspan();
			ps_desc.ps = ps_blob.cspan();
			ps_desc.shader_input_layout = m_lighting_pass_slayout;
			ps_desc.num_render_targets = 1;
			ps_desc.rtv_formats[0] = Format::rgba32_float;
			ps_desc.dsv_format = Format::d32_float;
			luset(m_lighting_pass_pso, device->new_graphic_pipeline_state(ps_desc));

            luset(m_default_base_color, device->new_resource(
				ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_roughness, device->new_resource(
				ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::r8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_normal, device->new_resource(
				ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_metallic, device->new_resource(
				ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::r8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_emissive, device->new_resource(
				ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));

			// Upload default texture data.
			luexp(m_default_base_color->map_subresource(0, false));
			luexp(m_default_roughness->map_subresource(0, false));
			luexp(m_default_normal->map_subresource(0, false));
			luexp(m_default_metallic->map_subresource(0, false));
			luexp(m_default_emissive->map_subresource(0, false));
			u8 data[4] = { 255, 255, 255, 255 };
			luexp(m_default_base_color->write_subresource(0, data, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 127;
			luexp(m_default_roughness->write_subresource(0, data, 1, 1, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 127;
			data[1] = 127;
			data[2] = 255;
			data[3] = 255;
			luexp(m_default_normal->write_subresource(0, data, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 0;
			luexp(m_default_metallic->write_subresource(0, data, 1, 1, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 0;
			data[1] = 0;
			data[2] = 0;
			data[3] = 0;
			luexp(m_default_emissive->write_subresource(0, data, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
			m_default_base_color->unmap_subresource(0, true);
			m_default_roughness->unmap_subresource(0, true);
			m_default_normal->unmap_subresource(0, true);
			m_default_metallic->unmap_subresource(0, true);
			m_default_emissive->unmap_subresource(0, true);
            

        }
        lucatchret;
        return ok;
    }

    RV LightingPass::init(LightingPassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }

    RV LightingPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto scene_tex = ctx->get_output("scene_texture");
            auto depth_tex = ctx->get_output("scene_depth_texture");
            auto render_desc = scene_tex->get_desc();
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_constant_buffer_data_alignment();
            lulet(scene_tex_rtv, device->new_render_target_view(scene_tex));
            lulet(depth_dsv, device->new_depth_stencil_view(depth_tex));
            //auto fbo = device->new_frame_buffer(m_global_data->m_lighting_pass_rp, 1, &lighting_rt, nullptr, depth_tex, nullptr).get();
			cmdbuf->resource_barriers({ 
				ResourceBarrierDesc::as_transition(scene_tex, ResourceState::render_target),
				ResourceBarrierDesc::as_transition(depth_tex, ResourceState::depth_stencil_write) });
			RenderPassDesc render_pass;
			render_pass.rtvs[0] = scene_tex_rtv;
			render_pass.dsv = depth_dsv;
			render_pass.depth_load_op = RHI::LoadOp::clear;
			render_pass.depth_clear_value = 1.0f;
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->set_graphic_shader_input_layout(m_global_data->m_lighting_pass_slayout);
			cmdbuf->set_pipeline_state(m_global_data->m_lighting_pass_pso);
			cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
			cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width_or_buffer_size, (f32)render_desc.height, 0.0f, 1.0f));
			cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width_or_buffer_size, (i32)render_desc.height));

			// Draw Meshes.
			for (usize i = 0; i < ts.size(); ++i)
			{
				auto model = Asset::get_asset_data<Model>(rs[i]->model);
				auto mesh = Asset::get_asset_data<Mesh>(model->mesh);
				cmdbuf->set_vertex_buffers(0, { VertexBufferViewDesc(mesh->vb, 0,
					mesh->vb_count * sizeof(Vertex), sizeof(Vertex)) });
				cmdbuf->set_index_buffer(mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint);

				u32 num_pieces = (u32)mesh->pieces.size();

				for (u32 j = 0; j < num_pieces; ++j)
				{
					Ref<RHI::IResource> base_color_tex = m_global_data->m_default_base_color;
					Ref<RHI::IResource> roughness_tex = m_global_data->m_default_roughness;
					Ref<RHI::IResource> normal_tex = m_global_data->m_default_normal;
					Ref<RHI::IResource> metallic_tex = m_global_data->m_default_metallic;
					Ref<RHI::IResource> emissive_tex = m_global_data->m_default_emissive;
					Ref<RHI::IResource> sky_tex = m_global_data->m_default_emissive;

					if (j < model->materials.size())
					{
						auto mat = Asset::get_asset_data<Material>(model->materials[j]);
						if (mat)
						{
							// Set material for this piece.
							Ref<RHI::IResource> mat_base_color_tex = Asset::get_asset_data<RHI::IResource>(mat->base_color);
							Ref<RHI::IResource> mat_roughness_tex = Asset::get_asset_data<RHI::IResource>(mat->roughness);
							Ref<RHI::IResource> mat_normal_tex = Asset::get_asset_data<RHI::IResource>(mat->normal);
							Ref<RHI::IResource> mat_metallic_tex = Asset::get_asset_data<RHI::IResource>(mat->metallic);
							Ref<RHI::IResource> mat_emissive_tex = Asset::get_asset_data<RHI::IResource>(mat->emissive);
							if (mat_base_color_tex)
							{
								base_color_tex = mat_base_color_tex;
							}
							if (mat_roughness_tex)
							{
								roughness_tex = mat_roughness_tex;
							}
							if (mat_normal_tex)
							{
								normal_tex = mat_normal_tex;
							}
							if (mat_metallic_tex)
							{
								metallic_tex = mat_metallic_tex;
							}
							if (mat_emissive_tex)
							{
								emissive_tex = mat_emissive_tex;
							}
						}
					}

					if (skybox) sky_tex = skybox;
					lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_lighting_pass_dlayout)));
					vs->set_cbv(0, camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
					vs->set_srv(1, model_matrices, &ShaderResourceViewDesc::as_buffer(i, 1, sizeof(Float4x4) * 2, false));
					if (light_ts.empty())
					{
						// Adds one fake light.
						vs->set_srv(2, light_params, &ShaderResourceViewDesc::as_buffer(0, 1, sizeof(LightingParams)));
					}
					else
					{
						vs->set_srv(2, light_params, &ShaderResourceViewDesc::as_buffer(0, (u32)light_ts.size(), sizeof(LightingParams)));
					}
					// Set material texture: base_color(t2), roughness(t3), normal(t4), metallic(t5), emissive(t6).
					vs->set_srv(3, base_color_tex);
					vs->set_srv(4, roughness_tex);
					vs->set_srv(5, normal_tex);
					vs->set_srv(6, metallic_tex);
					vs->set_srv(7, emissive_tex);
					vs->set_srv(8, sky_tex);
					vs->set_sampler(9, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
					cmdbuf->set_graphic_descriptor_set(0, vs);
					cmdbuf->attach_graphic_object(vs);
					cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
				}
			}
			cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }
    RV compile_lighting_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            LightingPassGlobalData* data = (LightingPassGlobalData*)userdata;
			auto scene_texture = compiler->get_output_resource("scene_texture");
			auto scene_depth_texture = compiler->get_output_resource("scene_depth_texture");
			if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "LightingPass: Output \"scene_texture\" is not specified.");
			if(scene_depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "LightingPass: Output \"scene_depth_texture\" is not specified.");
			RHI::ResourceDesc desc = compiler->get_resource_desc(scene_texture);
			if (desc.pixel_format != RHI::Format::rgba32_float)
			{
				return set_error(BasicError::bad_arguments(), "LightingPass: Invalid format for \"scene_texture\" is specified. \"scene_texture\" must be Format::rgba32_float.");
			}
			desc.usages |= RHI::ResourceUsageFlag::render_target;
			compiler->set_resource_desc(scene_texture, desc);
			desc = compiler->get_resource_desc(scene_depth_texture);
			if (desc.pixel_format != RHI::Format::d32_float)
			{
				return set_error(BasicError::bad_arguments(), "LightingPass: Invalid format for \"scene_depth_texture\" is specified. \"scene_depth_texture\" must be Format::d32_float.");
			}
			desc.usages |= RHI::ResourceUsageFlag::depth_stencil;
			compiler->set_resource_desc(scene_depth_texture, desc);
			Ref<LightingPass> pass = new_object<LightingPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_lighting_pass()
    {
        lutry
        {
            register_boxed_type<LightingPassGlobalData>();
            register_boxed_type<LightingPass>();
            impl_interface_for_type<LightingPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Lighting";
            desc.desc = "Illuminate the scene.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture."});
            desc.output_parameters.push_back({"scene_depth_texture", "The scene depth texture"});
            desc.compile = compile_lighting_pass;
            auto data = new_object<LightingPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}
