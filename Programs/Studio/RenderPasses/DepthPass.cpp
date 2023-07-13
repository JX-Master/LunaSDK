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
#include <Luna/Runtime/File.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include "../SceneRenderer.hpp"
#include "../Material.hpp"
#include "../StudioHeader.hpp"
#include <Luna/RHI/Utility.hpp>

namespace Luna
{
    RV DepthPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_depth_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::vertex),
				DescriptorSetLayoutBinding(DescriptorType::read_buffer_view, 1, 1, ShaderVisibilityFlag::vertex),
				DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 2, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibilityFlag::pixel),
				})));
			auto dl = m_depth_pass_dlayout.get();
			luset(m_depth_pass_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dl, 1 },
				PipelineLayoutFlag::allow_input_assembler_input_layout)));

			lulet(vs_blob, compile_shader("Shaders/DepthVert.hlsl", ShaderCompiler::ShaderType::vertex));
			lulet(ps_blob, compile_shader("Shaders/DepthPixel.hlsl", ShaderCompiler::ShaderType::pixel));

			GraphicsPipelineStateDesc ps_desc;
			ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
			ps_desc.sample_mask = U32_MAX;
			ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(false, BlendFactor::src_alpha,
				BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
			ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
			ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
			ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			Vector<InputAttributeDesc> attributes;
			get_vertex_input_layout_desc(attributes);
			InputBindingDesc binding(0, sizeof(Vertex), InputRate::per_vertex);
			ps_desc.input_layout.attributes = { attributes.data(), attributes.size() };
			ps_desc.input_layout.bindings = { &binding, 1 };
			ps_desc.vs = vs_blob.cspan();
			ps_desc.ps = ps_blob.cspan();
			ps_desc.pipeline_layout = m_depth_pass_playout;
			ps_desc.num_color_attachments = 0;
			ps_desc.depth_stencil_format = Format::d32_float;
			luset(m_depth_pass_pso, device->new_graphics_pipeline_state(ps_desc));

			luset(m_default_base_color, device->new_texture(MemoryType::local,
				TextureDesc::tex2d(Format::rgba8_unorm, TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, 1, 1, 1, 1)));
			u8 base_color_data[4] = { 255, 255, 255, 255 };
			lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
			luexp(copy_resource_data(upload_cmdbuf, {CopyResourceData::write_texture(m_default_base_color, SubresourceIndex(0, 0), 0, 0, 0, base_color_data, 4, 4, 1, 1, 1)}));
        }
        lucatchret;
        return ok;
    }

    RV DepthPass::init(DepthPassGlobalData* global_data)
    {
		m_global_data = global_data;
        return ok;
    }

    RV DepthPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            Ref<ITexture> depth_tex = ctx->get_output("depth_texture");
            auto render_desc = depth_tex->get_desc();
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_uniform_buffer_data_alignment();
			cmdbuf->resource_barrier({}, {
				{depth_tex, SubresourceIndex(0, 0), TextureStateFlag::none, TextureStateFlag::depth_stencil_attachment_write, ResourceBarrierFlag::discard_content } });
			RenderPassDesc render_pass;
			render_pass.depth_stencil_attachment = DepthStencilAttachment(depth_tex, false, LoadOp::clear, StoreOp::store, 1.0f);
			cmdbuf->set_context(CommandBufferContextType::graphics);
			for (usize i = 0; i < ts.size(); ++i)
			{
				auto model = Asset::get_asset_data<Model>(rs[i]->model);
				auto mesh = Asset::get_asset_data<Mesh>(model->mesh);
				cmdbuf->set_vertex_buffers(0, { VertexBufferView(mesh->vb, 0,
					mesh->vb_count * sizeof(Vertex), sizeof(Vertex)) });
				cmdbuf->set_index_buffer({ mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint });

				u32 num_pieces = (u32)mesh->pieces.size();

				for (u32 j = 0; j < num_pieces; ++j)
				{
					Ref<RHI::ITexture> base_color_tex = m_global_data->m_default_base_color;
					if (j < model->materials.size())
					{
						auto mat = Asset::get_asset_data<Material>(model->materials[j]);
						if (mat)
						{
							// Set material for this piece.
							Ref<RHI::ITexture> mat_base_color_tex = Asset::get_asset_data<RHI::ITexture>(mat->base_color);
							if (mat_base_color_tex)
							{
								base_color_tex = mat_base_color_tex;
							}
						}
					}
					cmdbuf->resource_barrier({}, {
							TextureBarrier(base_color_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps)
						});
				}
			}
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->set_graphics_pipeline_layout(m_global_data->m_depth_pass_playout);
			cmdbuf->set_graphics_pipeline_state(m_global_data->m_depth_pass_pso);
			cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width, (f32)render_desc.height, 0.0f, 1.0f));
			cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width, (i32)render_desc.height));

			// Draw Meshes.
			for (usize i = 0; i < ts.size(); ++i)
			{
				auto model = Asset::get_asset_data<Model>(rs[i]->model);
				auto mesh = Asset::get_asset_data<Mesh>(model->mesh);
				cmdbuf->set_vertex_buffers(0, { VertexBufferView(mesh->vb, 0,
					mesh->vb_count * sizeof(Vertex), sizeof(Vertex)) });
				cmdbuf->set_index_buffer({mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint});

				u32 num_pieces = (u32)mesh->pieces.size();

				for (u32 j = 0; j < num_pieces; ++j)
				{
					Ref<RHI::ITexture> base_color_tex = m_global_data->m_default_base_color;

					if (j < model->materials.size())
					{
						auto mat = Asset::get_asset_data<Material>(model->materials[j]);
						if (mat)
						{
							// Set material for this piece.
							Ref<RHI::ITexture> mat_base_color_tex = Asset::get_asset_data<RHI::ITexture>(mat->base_color);
							if (mat_base_color_tex)
							{
								base_color_tex = mat_base_color_tex;
							}
						}
					}

					lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_depth_pass_dlayout)));
					vs->update_descriptors({
						WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(camera_cb, 0, (u32)align_upper(sizeof(CameraCB), cb_align))),
						WriteDescriptorSet::read_buffer_view(1, BufferViewDesc::structured_buffer(model_matrices, i, 1, sizeof(Float4x4) * 2)),
						WriteDescriptorSet::read_texture_view(2, TextureViewDesc::tex2d(base_color_tex)),
						WriteDescriptorSet::sampler(3, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat))
						});
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
			RG::ResourceDesc desc = compiler->get_resource_desc(depth_texture);
			if (desc.texture.format != RHI::Format::d32_float)
			{
				return set_error(BasicError::bad_arguments(), "DepthPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be Format::d32_float.");
			}
			desc.texture.usages |= RHI::TextureUsageFlag::depth_stencil_attachment;
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