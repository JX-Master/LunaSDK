/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file NormalVisualizationPass.cpp
* @author JXMaster
* @date 2023/4/14
*/
#include "NormalVisualizationPass.hpp"
#include <Runtime/File.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include "../Material.hpp"
#include "../SceneRenderer.hpp"
#include "CommonVertex.hpp"

namespace Luna
{
    RV NormalVisualizationPassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_normal_visualization_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
				DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all)
				})));

			luset(m_normal_visualization_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_normal_visualization_pass_dlayout },
				ShaderInputLayoutFlag::allow_input_assembler_input_layout |
				ShaderInputLayoutFlag::deny_domain_shader_access |
				ShaderInputLayoutFlag::deny_hull_shader_access)));

			lulet(vsf, open_file("NormalVisualizationVert.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = vsf->get_size();
			auto vs_blob = Blob((usize)file_size);
			luexp(vsf->read(vs_blob.span()));
			vsf = nullptr;

            lulet(gsf, open_file("NormalVisualizationGeo.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			file_size = gsf->get_size();
			auto gs_blob = Blob((usize)file_size);
			luexp(gsf->read(gs_blob.span()));
			gsf = nullptr;

			lulet(psf, open_file("NormalVisualizationPixel.cso", FileOpenFlag::read, FileCreationMode::open_existing));
			file_size = psf->get_size();
			auto ps_blob = Blob((usize)file_size);
			luexp(psf->read(ps_blob.span()));
			psf = nullptr;

			GraphicsPipelineStateDesc ps_desc;
			ps_desc.primitive_topology_type = PrimitiveTopologyType::point;
			ps_desc.sample_mask = U32_MAX;
			ps_desc.sample_quality = 0;
			ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(true, false, BlendFactor::src_alpha,
				BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::one, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
			ps_desc.rasterizer_state = RasterizerDesc(FillMode::wireframe, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
			ps_desc.depth_stencil_state = DepthStencilDesc(true, false, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
			ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			ps_desc.input_layout = get_vertex_input_layout_desc();
			ps_desc.vs = vs_blob.cspan();
            ps_desc.gs = gs_blob.cspan();
			ps_desc.ps = ps_blob.cspan();
			ps_desc.shader_input_layout = m_normal_visualization_pass_slayout;
			ps_desc.num_render_targets = 1;
			ps_desc.rtv_formats[0] = Format::rgba8_unorm;
			ps_desc.dsv_format = Format::d32_float;
			luset(m_normal_visualization_pass_pso, device->new_graphics_pipeline_state(ps_desc));
        }
        lucatchret;
        return ok;
    }

    RV NormalVisualizationPass::init(NormalVisualizationPassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }

    RV NormalVisualizationPass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto scene_tex = ctx->get_output("scene_texture");
			auto depth_tex = ctx->get_input("depth_texture");
            auto render_desc = scene_tex->get_desc();
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_constant_buffer_data_alignment();
            lulet(scene_tex_rtv, device->new_render_target_view(scene_tex));
            lulet(depth_dsv, device->new_depth_stencil_view(depth_tex));
            //auto fbo = device->new_frame_buffer(m_global_data->m_lighting_pass_rp, 1, &lighting_rt, nullptr, depth_tex, nullptr).get();
			cmdbuf->resource_barriers({ 
				ResourceBarrierDesc::as_transition(scene_tex, ResourceState::render_target),
				ResourceBarrierDesc::as_transition(depth_tex, ResourceState::depth_stencil_read) });
			RenderPassDesc render_pass;
			render_pass.rtvs[0] = scene_tex_rtv;
			render_pass.rt_load_ops[0] = LoadOp::load;
			render_pass.dsv = depth_dsv;
			render_pass.depth_load_op = RHI::LoadOp::load;
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->set_graphics_shader_input_layout(m_global_data->m_normal_visualization_pass_slayout);
			cmdbuf->set_pipeline_state(m_global_data->m_normal_visualization_pass_pso);
			cmdbuf->set_primitive_topology(PrimitiveTopology::point_list);
			cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width_or_buffer_size, (f32)render_desc.height, 0.0f, 1.0f));
			cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width_or_buffer_size, (i32)render_desc.height));

			// Draw Meshes.
			for (usize i = 0; i < ts.size(); ++i)
			{
				auto model = Asset::get_asset_data<Model>(rs[i]->model);
				auto mesh = Asset::get_asset_data<Mesh>(model->mesh);
				cmdbuf->set_vertex_buffers(0, { VertexBufferViewDesc(mesh->vb, 0,
					mesh->vb_count * sizeof(Vertex), sizeof(Vertex)) });
				lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_normal_visualization_pass_dlayout)));
				vs->set_cbv(0, camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
				vs->set_srv(1, model_matrices, &ShaderResourceViewDesc::as_buffer(Format::unknown, i, 1, sizeof(Float4x4) * 2, false));
				cmdbuf->set_graphics_descriptor_set(0, vs);
				cmdbuf->attach_device_object(vs);
                cmdbuf->draw(mesh->vb_count, 0);
			}
			cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }
	RV compile_normal_visualization_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            NormalVisualizationPassGlobalData* data = (NormalVisualizationPassGlobalData*)userdata;
            auto scene_texture = compiler->get_output_resource("scene_texture");
			auto depth_texture = compiler->get_input_resource("depth_texture");
			if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "NormalVisualizationPass: Output \"scene_texture\" is not specified.");
			if(depth_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "NormalVisualizationPass: Input \"depth_texture\" is not specified.");
			RHI::ResourceDesc desc = compiler->get_resource_desc(depth_texture);
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

			auto desc2 = compiler->get_resource_desc(scene_texture);
			desc2.type = RHI::ResourceType::texture_2d;
			if(!desc2.width_or_buffer_size) desc2.width_or_buffer_size = desc.width_or_buffer_size;
			if(!desc2.height) desc2.height = desc.height;
			if(desc2.pixel_format == RHI::Format::unknown) desc2.pixel_format = RHI::Format::rgba8_unorm;
			desc2.usages |= RHI::ResourceUsageFlag::render_target;
			compiler->set_resource_desc(scene_texture, desc2);

			Ref<NormalVisualizationPass> pass = new_object<NormalVisualizationPass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }
    RV register_normal_visualization_pass()
    {
        lutry
        {
            register_boxed_type<NormalVisualizationPassGlobalData>();
            register_boxed_type<NormalVisualizationPass>();
            impl_interface_for_type<NormalVisualizationPass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "NormalVisualization";
            desc.desc = "Draws vertex normal to the target.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture to draw the normal lines to."});
            desc.input_parameters.push_back({"depth_texture", "The scene depth texture with pre-rendered depth information."});
            desc.compile = compile_normal_visualization_pass;
            auto data = new_object<NormalVisualizationPassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
    }
}