/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2022/8/2
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Runtime/Runtime.hpp>
#include <Runtime/Module.hpp>
#include <Runtime/Debug.hpp>
#include <Runtime/Math/Color.hpp>
#include <ShaderCompiler/ShaderCompiler.hpp>
#include <Runtime/Log.hpp>
#include <RHI/ShaderCompileHelper.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

Ref<RHI::IShaderInputLayout> shader_input_layout;
Ref<RHI::IPipelineState> pso;
Ref<RHI::IBuffer> vb;

struct VertexData
{
	Float2U pos;
	Float4U color;
};

RV start()
{
	lutry
	{
		// create pso
		{
			const char vs_shader_code[] =
				R"(
				struct VS_INPUT
				{
					[[vk::location(0)]]
					float2 pos : POSITION;
					[[vk::location(1)]]
					float4 col : COLOR0;
				};
				struct PS_INPUT
				{
					[[vk::location(0)]]
					float4 pos : SV_POSITION;
					[[vk::location(1)]]
					float4 col  : COLOR0;
				};
				PS_INPUT main(VS_INPUT input)
				{
					PS_INPUT output;
					output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
					output.col  = input.col;
					return output;
				})";
			auto compiler = ShaderCompiler::new_compiler();
			compiler->set_source({ vs_shader_code, sizeof(vs_shader_code) });
			compiler->set_source_name("TestTriangleVS");
			compiler->set_entry_point("main");
			compiler->set_target_format(RHI::get_current_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
			compiler->set_shader_model(5, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);

			luexp(compiler->compile());

			auto vs_data = compiler->get_output();
			Blob vs(vs_data.data(), vs_data.size());

			const char ps_shader_code[] =
				R"(struct PS_INPUT
				{
					[[vk::location(0)]]
					float4 pos : SV_POSITION;
					[[vk::location(1)]]
					float4 col : COLOR0;
				};
				[[vk::location(0)]]
				float4 main(PS_INPUT input) : SV_Target
				{
					return input.col;
				})";
			compiler->reset();
			compiler->set_source({ ps_shader_code, sizeof(ps_shader_code) });
			compiler->set_source_name("TestTrianglePS");
			compiler->set_entry_point("main");
			compiler->set_target_format(RHI::get_current_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
			compiler->set_shader_model(5, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);

			luexp(compiler->compile());

			auto ps_data = compiler->get_output();
			Blob ps(ps_data.data(), ps_data.size());

			luset(shader_input_layout, get_main_device()->new_shader_input_layout(ShaderInputLayoutDesc({},
				ShaderInputLayoutFlag::allow_input_assembler_input_layout |
				ShaderInputLayoutFlag::deny_pixel_shader_access |
				ShaderInputLayoutFlag::deny_vertex_shader_access)));

			GraphicsPipelineStateDesc desc;
			desc.input_layout = InputLayoutDesc({
				{
					InputInstanceDesc(0, sizeof(VertexData), InputRate::per_vertex)
				},
				{
					InputPropertyDesc("POSITION", 0, 0, 0, 0, Format::rg32_float),
					InputPropertyDesc("COLOR", 0, 0, 1, 8, Format::rgba32_float)
				}
			});
			desc.shader_input_layout = shader_input_layout;
			desc.vs = { vs.data(), vs.size() };
			desc.ps = { ps.data(), ps.size() };
			desc.rasterizer_state.depth_clip_enable = false;
			desc.depth_stencil_state = DepthStencilDesc(false, false);
			desc.num_render_targets = 1;
			desc.rtv_formats[0] = Format::bgra8_unorm;

			luset(pso, get_main_device()->new_graphics_pipeline_state(desc));

			// prepare draw buffer. POSITION : COLOR
			VertexData data[3];
			if (RHI::get_current_platform_api_type() == APIType::vulkan)
			{
				data[0] = { { 0.0f, -0.7f},{1.0f, 0.0f, 0.0f, 1.0f} };
				data[1] = { { 0.7f, 0.7f},{0.0f, 1.0f, 0.0f, 1.0f} };
				data[2] = { {-0.7f, 0.7f},{0.0f, 0.0f, 1.0f, 1.0f} };
			}
			else
			{
				data[0] = { { 0.0f,  0.7f},{1.0f, 0.0f, 0.0f, 1.0f} };
				data[1] = { { 0.7f, -0.7f},{0.0f, 1.0f, 0.0f, 1.0f} };
				data[2] = { {-0.7f, -0.7f},{0.0f, 0.0f, 1.0f, 1.0f} };
			}

			luset(vb, get_main_device()->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::vertex_buffer, sizeof(data))));
			lulet(mapped_data, vb->map(0, 0));
			memcpy(mapped_data, data, sizeof(data));
			vb->unmap(0, sizeof(data));
		}
	}
	lucatchret;
	return ok;
}

void draw()
{
	auto rtv = get_main_device()->new_render_target_view(get_back_buffer()).get();
	auto cb = get_command_buffer();
	cb->resource_barrier({}, {
			{get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
		});
	cb->attach_device_object(rtv);
	RenderPassDesc desc;
	desc.color_attachments[0] = rtv;
	desc.color_load_ops[0] = LoadOp::clear;
	desc.color_clear_values[0] = Color::yellow();
	desc.color_store_ops[0] = StoreOp::store;
	cb->begin_render_pass(desc);
	cb->set_pipeline_state(PipelineStateBindPoint::graphics, pso);
	cb->set_graphics_shader_input_layout(shader_input_layout);
	IBuffer* vertex_buffer = vb;
	usize vb_offset = 0;
	cb->set_vertex_buffers(0, {VertexBufferView(vb, 0, sizeof(VertexData) * 3, sizeof(VertexData))});
	auto sz = get_window()->get_size();
	cb->set_scissor_rect(RectI(0, 0, (i32)sz.x, (i32)sz.y));
	cb->set_viewport(Viewport(0.0f, 0.0f, (f32)sz.x, (f32)sz.y, 0.0f, 1.0f));
	cb->draw(3, 0);
	cb->end_render_pass();
	cb->resource_barrier({},
		{
			{get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::color_attachment_write, TextureStateFlag::present, ResourceBarrierFlag::none}
		});
	lupanic_if_failed(cb->submit({}, {}, true));
	cb->wait();
}

void resize(u32 width, u32 height)
{
}

void cleanup()
{
	shader_input_layout.reset();
	pso.reset();
	vb.reset();
}

void run_app()
{
	register_init_func(start);
	register_close_func(cleanup);
	register_resize_func(resize);
	register_draw_func(draw);
	lupanic_if_failed(run());
}

int main()
{
	if (!Luna::init()) return 0;
	auto r = init_modules();
	if (failed(r))
	{
		log_error("%s", explain(r.errcode()));
	}
	else run_app();
	Luna::close();
	return 0;
}