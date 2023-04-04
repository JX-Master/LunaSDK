/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file DepthPass.cpp
* @author JXMaster
* @date 2023/3/11
*/
#include "DepthPass.hpp"
#include <Runtime/File.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include "../SceneRenderer.hpp"
#include "../Material.hpp"

namespace Luna
{
    RV DepthPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_depth_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::vertex),
				DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::vertex),
				DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibility::pixel),
				})));

			luset(m_depth_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_depth_pass_dlayout },
				ShaderInputLayoutFlag::allow_input_assembler_input_layout |
				ShaderInputLayoutFlag::deny_domain_shader_access |
				ShaderInputLayoutFlag::deny_geometry_shader_access |
				ShaderInputLayoutFlag::deny_hull_shader_access)));

			lulet(vsf, open_file("DepthVert.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = vsf->get_size();
			auto vs_blob = Blob((usize)file_size);
			luexp(vsf->read(vs_blob.span()));
			vsf = nullptr;

			lulet(psf, open_file("DepthPixel.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			file_size = psf->get_size();
			auto ps_blob = Blob((usize)file_size);
			luexp(psf->read(ps_blob.span()));
			psf = nullptr;

			GraphicsPipelineStateDesc ps_desc;
			ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
			ps_desc.sample_mask = U32_MAX;
			ps_desc.sample_quality = 0;
			ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(false, false, BlendFactor::src_alpha,
				BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
			ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
			ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
			ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			ps_desc.input_layout = get_vertex_input_layout_desc();
			ps_desc.vs = vs_blob.cspan();
			ps_desc.ps = ps_blob.cspan();
			ps_desc.shader_input_layout = m_depth_pass_slayout;
			ps_desc.num_render_targets = 0;
			ps_desc.dsv_format = Format::d32_float;
			luset(m_depth_pass_pso, device->new_graphics_pipeline_state(ps_desc));

			luset(m_default_base_color, device->new_resource(
				ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			u8 base_color_data[4] = { 255, 255, 255, 255 };
			luexp(device->copy_resource({
				ResourceCopyDesc::as_write_texture(m_default_base_color, base_color_data, 4, 4, 0, BoxU(0, 0, 0, 1, 1, 1))}));
        }
        lucatchret;
        return ok;
    }

    RV DepthPass::init(DepthPassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }

    RV DepthPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto depth_tex = ctx->get_output("depth_texture");
            auto render_desc = depth_tex->get_desc();
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_constant_buffer_data_alignment();
            lulet(depth_dsv, device->new_depth_stencil_view(depth_tex));
            //auto fbo = device->new_frame_buffer(m_global_data->m_lighting_pass_rp, 1, &lighting_rt, nullptr, depth_tex, nullptr).get();
			cmdbuf->resource_barriers({ 
				ResourceBarrierDesc::as_transition(depth_tex, ResourceState::depth_stencil_write) });
			RenderPassDesc render_pass;
			render_pass.dsv = depth_dsv;
			render_pass.depth_load_op = RHI::LoadOp::clear;
			render_pass.depth_clear_value = 1.0f;
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->set_graphics_shader_input_layout(m_global_data->m_depth_pass_slayout);
			cmdbuf->set_pipeline_state(m_global_data->m_depth_pass_pso);
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
				cmdbuf->set_index_buffer({mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint});

				u32 num_pieces = (u32)mesh->pieces.size();

				for (u32 j = 0; j < num_pieces; ++j)
				{
					Ref<RHI::IResource> base_color_tex = m_global_data->m_default_base_color;

					if (j < model->materials.size())
					{
						auto mat = Asset::get_asset_data<Material>(model->materials[j]);
						if (mat)
						{
							// Set material for this piece.
							Ref<RHI::IResource> mat_base_color_tex = Asset::get_asset_data<RHI::IResource>(mat->base_color);
							if (mat_base_color_tex)
							{
								base_color_tex = mat_base_color_tex;
							}
						}
					}

					lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_depth_pass_dlayout)));
					vs->set_cbv(0, camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
					vs->set_srv(1, model_matrices, &ShaderResourceViewDesc::as_buffer(Format::unknown, i, 1, sizeof(Float4x4) * 2, false));
					vs->set_srv(2, base_color_tex);
					vs->set_sampler(3, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
					cmdbuf->set_graphics_descriptor_set(0, vs);
					cmdbuf->attach_device_object(vs);
					cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
				}
			}
			cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }
    RV compile_depth_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            DepthPassGlobalData* data = (DepthPassGlobalData*)userdata;
			auto depth_texture = compiler->get_output_resource("depth_texture");
			if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "DepthPass: Output \"depth_texture\" is not specified.");
			RHI::ResourceDesc desc = compiler->get_resource_desc(depth_texture);
			if (desc.pixel_format != RHI::Format::d32_float)
			{
				return set_error(BasicError::bad_arguments(), "DepthPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be Format::d32_float.");
			}
			desc.usages |= RHI::ResourceUsageFlag::depth_stencil;
			compiler->set_resource_desc(depth_texture, desc);
			Ref<DepthPass> pass = new_object<DepthPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_depth_pass()
    {
        lutry
        {
            register_boxed_type<DepthPassGlobalData>();
            register_boxed_type<DepthPass>();
            impl_interface_for_type<DepthPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Depth";
            desc.desc = "Writes scene depth to the texture.";
            desc.output_parameters.push_back({"depth_texture", "The scene depth texture"});
            desc.compile = compile_depth_pass;
            auto data = new_object<DepthPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}