/*
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
#include "ImageData.hpp"
#include <Runtime/Log.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

Ref<RHI::IRenderTargetView> m_rtv;
Ref<RHI::IDescriptorSetLayout> m_desc_set_layout;
Ref<RHI::IShaderInputLayout> m_shader_input_layout;
Ref<RHI::IDescriptorSet> m_desc_set;
Ref<RHI::IPipelineState> m_pso;
Ref<RHI::IResource> m_tex;

Ref<RHI::IResource> m_vb;
Ref<RHI::IResource> m_ib;

struct VertexData
{
	Float2U pos;
	Float2U texcoord;
};

RV start()
{
	lutry
	{
		luset(m_rtv, get_main_device()->new_render_target_view(get_back_buffer()));

		// create pso
		{
			const char vs_shader_code[] =
				"\
						struct VS_INPUT\
						{\
						float2 pos : POSITION;\
						float2 uv : TEXCOORD0;\
						};\
						\
						struct PS_INPUT\
						{\
						float4 pos : SV_POSITION;\
						float2 uv  : TEXCOORD0;\
						};\
						\
						PS_INPUT main(VS_INPUT input)\
						{\
						PS_INPUT output;\
						output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);\
						output.uv  = input.uv;\
						return output;\
						}";

			auto compiler = ShaderCompiler::new_compiler();
			compiler->set_source({ vs_shader_code, sizeof(vs_shader_code) });
			compiler->set_source_name("TestTextureVS");
			compiler->set_entry_point("main");
			compiler->set_target_format(get_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
			compiler->set_shader_model(5, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);

			luexp(compiler->compile());

			auto vs_data = compiler->get_output();
			Blob vs(vs_data.data(), vs_data.size());

			const char ps_shader_code[] =
				"struct PS_INPUT\
							{\
							float4 pos : SV_POSITION;\
							float2 uv  : TEXCOORD0;\
							};\
							SamplerState sampler0 : register(s1);\
							Texture2D texture0 : register(t0);\
							\
							float4 main(PS_INPUT input) : SV_Target\
							{\
							float4 out_col = clamp(texture0.Sample(sampler0, input.uv), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)); \
							return out_col; \
							}";

			compiler->reset();
			compiler->set_source({ ps_shader_code, sizeof(ps_shader_code) });
			compiler->set_source_name("TestTexturePS");
			compiler->set_entry_point("main");
			compiler->set_target_format(get_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
			compiler->set_shader_model(5, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);

			luexp(compiler->compile());

			auto ps_data = compiler->get_output();
			Blob ps(ps_data.data(), ps_data.size());

			luset(m_desc_set_layout, get_main_device()->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::srv, 0, 1, ShaderVisibility::pixel),
				DescriptorSetLayoutBinding(DescriptorType::sampler, 1, 1, ShaderVisibility::pixel)
				})));

			luset(m_shader_input_layout, get_main_device()->new_shader_input_layout(ShaderInputLayoutDesc(
				{ m_desc_set_layout },
				ShaderInputLayoutFlag::allow_input_assembler_input_layout |
				ShaderInputLayoutFlag::deny_domain_shader_access | ShaderInputLayoutFlag::deny_geometry_shader_access |
				ShaderInputLayoutFlag::deny_hull_shader_access)));

			GraphicPipelineStateDesc desc;
			desc.input_layout = InputLayoutDesc({
				InputElementDesc("POSITION", 0, Format::rg32_float),
				InputElementDesc("TEXCOORD", 0, Format::rg32_float) });
			desc.vs = { vs.data(), vs.size() };
			desc.ps = { ps.data(), ps.size() };
			desc.shader_input_layout = m_shader_input_layout;
			desc.depth_stencil_state = DepthStencilDesc(false, false);
			desc.num_render_targets = 1;
			desc.rtv_formats[0] = Format::rgba8_unorm;

			luset(m_pso, get_main_device()->new_graphic_pipeline_state(desc));

			luset(m_vb, get_main_device()->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::vertex_buffer, sizeof(VertexData) * 4)));
			u32 incides[] = { 0, 1, 2, 1, 3, 2 };
			luset(m_ib, get_main_device()->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::index_buffer, sizeof(incides))));
			void* mapped_data;
			luexp(m_ib->map_subresource(0, false, &mapped_data));
			memcpy(mapped_data, incides, sizeof(incides));
			m_ib->unmap_subresource(0, true);

			// prepare texture - 128x128 with only 1 mip level.
			luset(m_tex, get_main_device()->new_resource(ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource,
				128, 128, 1, 1)));

			luexp(m_tex->map_subresource(0, false));

			BoxU box;
			box.offset_x = 0;
			box.offset_y = 0;
			box.offset_z = 0;
			box.width = 128;
			box.height = 128;
			box.depth = 1;
			luexp(m_tex->write_subresource(0, test_image_data_v, 128 * 4, 128 * 128 * 4, box));
			m_tex->unmap_subresource(0, true);

			luset(m_desc_set, get_main_device()->new_descriptor_set(DescriptorSetDesc(m_desc_set_layout)));

			m_desc_set->set_srv(0, m_tex, nullptr);
			
			m_desc_set->set_sampler(1, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::clamp,
				TextureAddressMode::clamp, TextureAddressMode::clamp, 0.0f, 1, ComparisonFunc::always, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 0.0f));
		}
	}
	lucatchret;
	return ok;
}

void draw()
{
	// prepare draw buffer. POSITION : TEXCOORD
		// 0----1
		// |    |
		// 2----3

	auto sz = get_window()->get_size();
	auto w = sz.x;
	auto h = sz.y;

	VertexData data[4]{
		{ { -128.0f / w,  128.0f / h },{ 0.0f, 0.0f } },
		{ {  128.0f / w,  128.0f / h },{ 1.0f, 0.0f } },
		{ { -128.0f / w, -128.0f / h },{ 0.0f, 1.0f } },
		{ {  128.0f / w, -128.0f / h },{ 1.0f, 1.0f } },
	};

	void* mapped;
	lupanic_if_failed(m_vb->map_subresource(0, false, &mapped));
	memcpy(mapped, data, sizeof(data));
	m_vb->unmap_subresource(0, true);

	auto cb = get_command_buffer();

	cb->resource_barrier(ResourceBarrierDesc::as_transition(get_back_buffer(), ResourceState::render_target, 0));
	RenderPassDesc desc;
	desc.rtvs[0] = m_rtv;
	desc.rt_load_ops[0] = LoadOp::clear;
	desc.rt_clear_values[0] = Color::black();
	cb->begin_render_pass(desc);
	cb->set_pipeline_state(m_pso);
	cb->set_graphic_shader_input_layout(m_shader_input_layout);
	cb->set_graphic_descriptor_set(0, m_desc_set);
	cb->set_primitive_topology(PrimitiveTopology::triangle_list);
	cb->set_vertex_buffers(0, { &VertexBufferViewDesc(m_vb, 0, sizeof(VertexData) * 4, sizeof(VertexData)), 1 });
	cb->set_index_buffer(m_ib, 0, sizeof(u32) * 6, Format::r32_uint);
	cb->set_scissor_rect(RectI(0, 0, (i32)w, (i32)h));
	cb->set_viewport(Viewport(0.0f, 0.0f, (f32)w, (f32)h, 0.0f, 1.0f));
	cb->draw_indexed(6, 0, 0);
	cb->end_render_pass();

	lupanic_if_failed(cb->submit());
	cb->wait();
}

void resize(u32 width, u32 height)
{
	m_rtv = get_main_device()->new_render_target_view(get_back_buffer()).get();
	draw();
}

void cleanup()
{
	m_rtv.reset();
	m_desc_set_layout.reset();
	m_shader_input_layout.reset();
	m_desc_set.reset();
	m_pso.reset();
	m_tex.reset();
	m_vb.reset();
	m_ib.reset();
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