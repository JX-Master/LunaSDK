/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file GeometryPass.cpp
* @author JXMaster
* @date 2023/3/11
*/
#include "GeometryPass.hpp"
#include <Runtime/File.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include "../Material.hpp"
#include "../SceneRenderer.hpp"

namespace Luna
{
    RV GeometryPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_geometry_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
				DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibilityFlag::all),
				DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 3, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 4, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 5, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::srv, 6, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::sampler, 7, 1, ShaderVisibilityFlag::pixel)
				})));

			luset(m_geometry_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_geometry_pass_dlayout },
				ShaderInputLayoutFlag::allow_input_assembler_input_layout)));

			lulet(vsf, open_file("GeometryVert.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = vsf->get_size();
			auto vs_blob = Blob((usize)file_size);
			luexp(vsf->read(vs_blob.span()));
			vsf = nullptr;

			lulet(psf, open_file("GeometryPixel.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			file_size = psf->get_size();
			auto ps_blob = Blob((usize)file_size);
			luexp(psf->read(ps_blob.span()));
			psf = nullptr;

			GraphicsPipelineStateDesc ps_desc;
			ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
			ps_desc.sample_mask = U32_MAX;
			ps_desc.sample_quality = 0;
			ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(false, false, BlendFactor::src_alpha,
				BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
			ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
			ps_desc.depth_stencil_state = DepthStencilDesc(true, false, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
			ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			ps_desc.input_layout = get_vertex_input_layout_desc();
			ps_desc.vs = vs_blob.cspan();
			ps_desc.ps = ps_blob.cspan();
			ps_desc.shader_input_layout = m_geometry_pass_slayout;
			ps_desc.num_color_attachments = 3;
			ps_desc.color_formats[0] = Format::rgba8_unorm;
            ps_desc.color_formats[1] = Format::rgba8_unorm;
            ps_desc.color_formats[2] = Format::rgba16_float;
			ps_desc.depth_stencil_format = Format::d32_float;
			luset(m_geometry_pass_pso, device->new_graphics_pipeline_state(ps_desc));

            luset(m_default_base_color, device->new_resource(
				ResourceDesc::tex2d(MemoryType::local, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_roughness, device->new_resource(
				ResourceDesc::tex2d(MemoryType::local, Format::r8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_normal, device->new_resource(
				ResourceDesc::tex2d(MemoryType::local, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_metallic, device->new_resource(
				ResourceDesc::tex2d(MemoryType::local, Format::r8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
			luset(m_default_emissive, device->new_resource(
				ResourceDesc::tex2d(MemoryType::local, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));

			// Upload default texture data.
			u8 base_color_data[4] = { 255, 255, 255, 255 };
			u8 roughness_data = 127;
			u8 normal_data[4] = { 127, 127, 255, 255 };
			u8 metallic_data = 0;
			u8 emissive_data[4] = { 0, 0, 0, 0 };

			luexp(device->copy_resource({
				ResourceCopyDesc::as_write_texture(m_default_base_color, base_color_data, 4, 4, 0, BoxU(0, 0, 0, 1, 1, 1)),
				ResourceCopyDesc::as_write_texture(m_default_roughness, &roughness_data, 1, 1, 0, BoxU(0, 0, 0, 1, 1, 1)),
				ResourceCopyDesc::as_write_texture(m_default_normal, normal_data, 4, 4, 0, BoxU(0, 0, 0, 1, 1, 1)),
				ResourceCopyDesc::as_write_texture(m_default_metallic, &metallic_data, 1, 1, 0, BoxU(0, 0, 0, 1, 1, 1)),
				ResourceCopyDesc::as_write_texture(m_default_emissive, emissive_data, 4, 4, 0, BoxU(0, 0, 0, 1, 1, 1)),
				}));
        }
        lucatchret;
        return ok;
    }

    RV GeometryPass::init(GeometryPassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }

    RV GeometryPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto base_color_roughness_tex = ctx->get_output("base_color_roughness_texture");
            auto normal_metallic_tex = ctx->get_output("normal_metallic_texture");
			auto emissive_tex = ctx->get_output("emissive_texture");
			auto depth_tex = ctx->get_input("depth_texture");
            auto render_desc = base_color_roughness_tex->get_desc();
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_uniform_buffer_data_alignment();
            lulet(base_color_rtv, device->new_render_target_view(base_color_roughness_tex));
			lulet(normal_rtv, device->new_render_target_view(normal_metallic_tex));
			lulet(emissive_rtv, device->new_render_target_view(emissive_tex));
            lulet(depth_dsv, device->new_depth_stencil_view(depth_tex));
            //auto fbo = device->new_frame_buffer(m_global_data->m_lighting_pass_rp, 1, &lighting_rt, nullptr, depth_tex, nullptr).get();
			cmdbuf->resource_barriers({ 
				ResourceBarrierDesc::as_transition(base_color_roughness_tex, ResourceStateFlag::render_target),
				ResourceBarrierDesc::as_transition(normal_metallic_tex, ResourceStateFlag::render_target),
				ResourceBarrierDesc::as_transition(emissive_tex, ResourceStateFlag::render_target),
				ResourceBarrierDesc::as_transition(depth_tex, ResourceStateFlag::depth_stencil_read) });
			RenderPassDesc render_pass;
			render_pass.color_attachments[0] = base_color_rtv;
			render_pass.color_attachments[1] = normal_rtv;
			render_pass.color_attachments[2] = emissive_rtv;
			render_pass.color_load_ops[0] = LoadOp::clear;
			render_pass.color_clear_values[0] = Float4U(0.0f);
			render_pass.color_load_ops[1] = LoadOp::clear;
			render_pass.color_clear_values[1] = Float4U(0.0f);
			render_pass.color_load_ops[2] = LoadOp::clear;
			render_pass.color_clear_values[2] = Float4U(0.0f);
			render_pass.dsv = depth_dsv;
			render_pass.depth_load_op = RHI::LoadOp::load;
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->set_graphics_shader_input_layout(m_global_data->m_geometry_pass_slayout);
			cmdbuf->set_pipeline_state(m_global_data->m_geometry_pass_pso);
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
					lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_geometry_pass_dlayout)));
					vs->set_cbv(0, camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
					vs->set_srv(1, model_matrices, &ShaderResourceViewDesc::as_buffer(Format::unknown, i, 1, sizeof(Float4x4) * 2, false));
					// Set material texture: base_color(t2), roughness(t3), normal(t4), metallic(t5), emissive(t6).
					vs->set_srv(2, base_color_tex);
					vs->set_srv(3, roughness_tex);
					vs->set_srv(4, normal_tex);
					vs->set_srv(5, metallic_tex);
					vs->set_srv(6, emissive_tex);
					vs->set_sampler(7, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
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
	RV compile_geometry_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            GeometryPassGlobalData* data = (GeometryPassGlobalData*)userdata;
			auto depth_texture = compiler->get_input_resource("depth_texture");
			if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Input \"depth_texture\" is not specified.");
			auto base_color_roughness_tex = compiler->get_output_resource("base_color_roughness_texture");
            auto normal_metallic_tex = compiler->get_output_resource("normal_metallic_texture");
			auto emissive_tex = compiler->get_output_resource("emissive_texture");
			if(base_color_roughness_tex == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"base_color_roughness_texture\" is not specified.");
			if(normal_metallic_tex == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"normal_metallic_tex\" is not specified.");
			if(emissive_tex == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "GeometryPass: Output \"emissive_tex\" is not specified.");
			RG::ResourceDesc desc = compiler->get_resource_desc(depth_texture);
			if(desc.type == RHI::ResourceType::texture_2d && desc.pixel_format == RHI::Format::unknown)
			{
				desc.pixel_format = RHI::Format::d32_float;
			}
			if (desc.type != RHI::ResourceType::texture_2d || desc.pixel_format != RHI::Format::d32_float)
			{
				return set_error(BasicError::bad_arguments(), "DepthPass: Invalid format for \"depth_texture\" is specified. \"depth_texture\" must be 2D texture with Format::d32_float.");
			}
			desc.usages |= RHI::ResourceUsageFlag::depth_stencil;
			compiler->set_resource_desc(depth_texture, desc);

			auto desc2 = compiler->get_resource_desc(base_color_roughness_tex);
			desc2.type = RHI::ResourceType::texture_2d;
			if(!desc2.width_or_buffer_size) desc2.width_or_buffer_size = desc.width_or_buffer_size;
			if(!desc2.height) desc2.height = desc.height;
			if(desc2.pixel_format == RHI::Format::unknown) desc2.pixel_format = RHI::Format::rgba8_unorm;
			desc2.usages |= RHI::TextureUsageFlag::color_attachment;
			compiler->set_resource_desc(base_color_roughness_tex, desc2);

			desc2 = compiler->get_resource_desc(normal_metallic_tex);
			desc2.type = RHI::ResourceType::texture_2d;
			if(!desc2.width_or_buffer_size) desc2.width_or_buffer_size = desc.width_or_buffer_size;
			if(!desc2.height) desc2.height = desc.height;
			if(desc2.pixel_format == RHI::Format::unknown) desc2.pixel_format = RHI::Format::rgba8_unorm;
			desc2.usages |= RHI::TextureUsageFlag::color_attachment;
			compiler->set_resource_desc(normal_metallic_tex, desc2);

			desc2 = compiler->get_resource_desc(emissive_tex);
			desc2.type = RHI::ResourceType::texture_2d;
			if(!desc2.width_or_buffer_size) desc2.width_or_buffer_size = desc.width_or_buffer_size;
			if(!desc2.height) desc2.height = desc.height;
			if(desc2.pixel_format == RHI::Format::unknown) desc2.pixel_format = RHI::Format::rgba16_float;
			desc2.usages |= RHI::TextureUsageFlag::color_attachment;
			compiler->set_resource_desc(emissive_tex, desc2);

			Ref<GeometryPass> pass = new_object<GeometryPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_geometry_pass()
    {
        lutry
        {
            register_boxed_type<GeometryPassGlobalData>();
            register_boxed_type<GeometryPass>();
            impl_interface_for_type<GeometryPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Geometry";
            desc.desc = "Writes scene geometry information to the geometry buffer (G-buffer).";
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture with pre-rendered depth information."});
			desc.output_parameters.push_back({"base_color_roughness_texture", "The base color (RGB) and roughness (A) G-buffer."});
			desc.output_parameters.push_back({"normal_metallic_texture", "The normal (RGB) and metallic (A) G-buffer."});
			desc.output_parameters.push_back({"emissive_texture", "The emissive (RGB) G-buffer."});
            desc.compile = compile_geometry_pass;
            auto data = new_object<GeometryPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}