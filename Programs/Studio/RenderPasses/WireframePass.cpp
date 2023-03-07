/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WireframePass.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include "WireframePass.hpp"
#include <ShaderCompiler/ShaderCompiler.hpp>
#include <RHI/ShaderCompileHelper.hpp>
#include "../Mesh.hpp"
#include "../Model.hpp"
#include <Asset/Asset.hpp>
#include "../Assets/SceneEditor.hpp"

namespace Luna
{
    RV WireframePassGlobalData::init(RHI::IDevice* device)
    {
        using namespace RHI;
        lutry
        {
            luset(m_debug_mesh_renderer_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::vertex),
				DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::vertex) })));

			luset(m_debug_mesh_renderer_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_debug_mesh_renderer_dlayout },
				ShaderInputLayoutFlag::allow_input_assembler_input_layout |
				ShaderInputLayoutFlag::deny_domain_shader_access |
				ShaderInputLayoutFlag::deny_geometry_shader_access |
				ShaderInputLayoutFlag::deny_hull_shader_access)));
			static const char* pixelShader =
				"struct PS_INPUT\
				{\
					float4 position : SV_POSITION;	\
					float3 normal : NORMAL;	\
					float3 tangent : TANGENT;	\
					float2 texcoord : TEXCOORD;	\
					float4 color : COLOR;	\
					float3 world_position : POSITION;	\
				}; \
				\
				float4 main(PS_INPUT input) : SV_Target\
				{\
				  return float4(1.0f, 1.0f, 1.0f, 1.0f); \
				}";
            auto compiler = ShaderCompiler::new_compiler();
			compiler->set_source({ pixelShader, strlen(pixelShader) });
			compiler->set_source_name("MeshDebugPS");
			compiler->set_entry_point("main");
			compiler->set_target_format(get_current_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
			compiler->set_shader_model(5, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
			luexp(compiler->compile());
			Blob ps_blob = compiler->get_output();

			GraphicPipelineStateDesc ps_desc;
			ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
			ps_desc.sample_mask = U32_MAX;
			ps_desc.sample_quality = 0;
			ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(true, false, BlendFactor::src_alpha,
				BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
			ps_desc.rasterizer_state = RasterizerDesc(FillMode::wireframe, CullMode::none, 0, 0.0f, 0.0f, 0, false, true, false, true, false);
			ps_desc.depth_stencil_state = DepthStencilDesc(false, false, ComparisonFunc::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
			ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			ps_desc.input_layout = m_common_vertex->input_layout_common;
			ps_desc.vs = m_common_vertex->vs_blob.cspan();
			ps_desc.ps = ps_blob.cspan();
			ps_desc.shader_input_layout = m_debug_mesh_renderer_slayout;
			ps_desc.num_render_targets = 1;
			ps_desc.rtv_formats[0] = Format::rgba8_unorm;
			luset(m_debug_mesh_renderer_pso, device->new_graphic_pipeline_state(ps_desc));
        }
        lucatchret;
        return ok;
    }
    RV WireframePass::init(WireframePassGlobalData* global_data)
    {
        lutry
        {
            m_global_data = global_data;
        }
        lucatchret;
        return ok;
    }
    RV WireframePass::execute(RG::IRenderPassContext* ctx)
    {
        using namespace RHI;
        lutry
        {
            auto cmdbuf = ctx->get_command_buffer();
            auto device = cmdbuf->get_device();
            auto cb_align = device->get_constant_buffer_data_alignment();
            auto output_tex = ctx->get_output("scene_texture");
            lulet(render_rtv, device->new_render_target_view(output_tex));
            // Debug wireframe pass.
			RenderPassDesc render_pass;
			render_pass.rtvs[0] = render_rtv;
			auto render_desc = output_tex->get_desc();
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->set_graphic_shader_input_layout(m_global_data->m_debug_mesh_renderer_slayout);
			cmdbuf->set_pipeline_state(m_global_data->m_debug_mesh_renderer_pso);
			cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
			cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width_or_buffer_size, (f32)render_desc.height, 0.0f, 1.0f));
			cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width_or_buffer_size, (i32)render_desc.height));
			// Draw Meshes.
			for (usize i = 0; i < ts.size(); ++i)
			{
				auto vs = device->new_descriptor_set(DescriptorSetDesc(m_global_data->m_debug_mesh_renderer_dlayout)).get();
				vs->set_cbv(0, camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
				vs->set_srv(1, model_matrices, &ShaderResourceViewDesc::as_buffer(i, 1, sizeof(Float4x4) * 2, false));
				cmdbuf->set_graphic_descriptor_set(0, vs);
				cmdbuf->attach_graphic_object(vs);

				// Draw pieces.
				auto mesh = Asset::get_asset_data<Mesh>(Asset::get_asset_data<Model>(rs[i]->model)->mesh);

				auto vb_view = VertexBufferViewDesc(mesh->vb, 0,
					mesh->vb_count * sizeof(Vertex), sizeof(Vertex));

				cmdbuf->set_vertex_buffers(0, { &vb_view, 1 });
				cmdbuf->set_index_buffer(mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint);

				u32 num_pieces = (u32)mesh->pieces.size();
				for (u32 j = 0; j < num_pieces; ++j)
				{
					cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
				}
			}
			cmdbuf->end_render_pass();
        }
        lucatchret;
        return ok;
    }

    RV compile_wireframe_pass(object_t userdata, RG::IRenderGraphCompiler* compiler)
    {
        lutry
        {
            WireframePassGlobalData* data = (WireframePassGlobalData*)userdata;
			auto scene_texture = compiler->get_output_resource("scene_texture");
			if(scene_texture == RG::INVALID_RESOURCE) return set_error(BasicError::bad_arguments(), "WireframePass: Output \"scene_texture\" is not specified.");
			if(!compiler->get_resource_desc(scene_texture, nullptr)) return set_error(BasicError::bad_arguments(), "WireframePass: The resource layout for output \"scene_texture\" is not specified.");
			Ref<WireframePass> pass = new_object<WireframePass>();
            luexp(pass->init(data));
			compiler->set_render_pass_object(pass);
        }
        lucatchret;
        return ok;
    }

	RV register_wireframe_pass()
	{
		lutry
        {
            register_boxed_type<WireframePassGlobalData>();
            register_boxed_type<WireframePass>();
            impl_interface_for_type<WireframePass, RG::IRenderPass>();
            RG::RenderPassTypeDesc desc;
            desc.name = "Wireframe";
            desc.desc = "Draws wireframe of the scene.";
            desc.output_parameters.push_back({"scene_texture", "The scene texture."});
            desc.compile = compile_wireframe_pass;
            auto data = new_object<WireframePassGlobalData>();
            luexp(data->init(RHI::get_main_device()));
            desc.userdata = data.object();
            RG::register_render_pass_type(desc);
        }
        lucatchret;
        return ok;
	}
}