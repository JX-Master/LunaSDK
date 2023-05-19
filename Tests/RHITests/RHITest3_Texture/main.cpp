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
#include <Runtime/Log.hpp>
#include <RHI/ShaderCompileHelper.hpp>
#include <Runtime/File.hpp>
#include <Image/Image.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

Ref<RHI::IDescriptorSetLayout> desc_set_layout;
Ref<RHI::IShaderInputLayout> shader_input_layout;
Ref<RHI::IDescriptorSet> desc_set;
Ref<RHI::IPipelineState> pso;
Ref<RHI::ITexture> tex;

Ref<RHI::IBuffer> vb;
Ref<RHI::IBuffer> ib;

u32 tex_width;
u32 tex_height;

struct VertexData
{
	Float2U pos;
	Float2U texcoord;
};

RV start()
{
	lutry
	{
		auto device = get_main_device();
		// create pso
		{
			const char vs_shader_code[] =
				R"(
				struct VS_INPUT
				{
					[[vk::location(0)]]
					float2 pos : POSITION;
					[[vk::location(1)]]
					float2 uv : TEXCOORD0;
				};
						
				struct PS_INPUT
				{
					[[vk::location(0)]]
					float4 pos : SV_POSITION;
					[[vk::location(1)]]
					float2 uv  : TEXCOORD0;
				};
						
				PS_INPUT main(VS_INPUT input)
				{
					PS_INPUT output;
					output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
					output.uv  = input.uv;
					return output;
				})";

			auto compiler = ShaderCompiler::new_compiler();
			compiler->set_source({ vs_shader_code, sizeof(vs_shader_code) });
			compiler->set_source_name("TestTextureVS");
			compiler->set_entry_point("main");
			compiler->set_target_format(RHI::get_current_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
			compiler->set_shader_model(6, 0);
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
					float2 uv  : TEXCOORD0;
				};
				SamplerState sampler0 : register(s1);
				Texture2D texture0 : register(t0);
				
				[[vk::location(0)]]
				float4 main(PS_INPUT input) : SV_Target
				{
					float4 out_col = clamp(texture0.Sample(sampler0, input.uv), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)); 
					return out_col; 
				})";

			compiler->reset();
			compiler->set_source({ ps_shader_code, sizeof(ps_shader_code) });
			compiler->set_source_name("TestTexturePS");
			compiler->set_entry_point("main");
			compiler->set_target_format(RHI::get_current_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
			compiler->set_shader_model(6, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);

			luexp(compiler->compile());

			auto ps_data = compiler->get_output();
			Blob ps(ps_data.data(), ps_data.size());

			luset(desc_set_layout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
				DescriptorSetLayoutBinding(DescriptorType::sampled_texture_view, 0, 1, ShaderVisibilityFlag::pixel),
				DescriptorSetLayoutBinding(DescriptorType::sampler, 1, 1, ShaderVisibilityFlag::pixel)
				})));

			IDescriptorSetLayout* ds_layout = desc_set_layout;

			luset(shader_input_layout, device->new_shader_input_layout(ShaderInputLayoutDesc(
				{ &ds_layout, 1 }, ShaderInputLayoutFlag::allow_input_assembler_input_layout)));

			GraphicsPipelineStateDesc desc;
			desc.input_layout = InputLayoutDesc({
				{
					InputBindingDesc(0, sizeof(VertexData), InputRate::per_vertex)
				},
				{
					InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rg32_float),
					InputAttributeDesc("TEXCOORD", 0, 1, 0, 8, Format::rg32_float)
				}
			});
			desc.vs = { vs.data(), vs.size() };
			desc.ps = { ps.data(), ps.size() };
			desc.shader_input_layout = shader_input_layout;
			desc.depth_stencil_state = DepthStencilDesc(false, false);
			desc.num_render_targets = 1;
			desc.rtv_formats[0] = Format::bgra8_unorm;

			luset(pso, device->new_graphics_pipeline_state(desc));

			luset(vb, device->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::vertex_buffer, sizeof(VertexData) * 4)));
			u32 incides[] = { 0, 1, 2, 1, 3, 2 };
			luset(ib, device->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::index_buffer, sizeof(incides))));
			lulet(mapped_data, ib->map(0, 0));
			memcpy(mapped_data, incides, sizeof(incides));
			ib->unmap(0, sizeof(incides));

			lulet(image_file, open_file("uv_checker.png", FileOpenFlag::read, FileCreationMode::open_existing));
			lulet(image_file_data, load_file_data(image_file));
			Image::ImageDesc image_desc;
			lulet(image_data, Image::read_image_file(image_file_data.data(), image_file_data.size(), Image::ImagePixelFormat::rgba8_unorm, image_desc));
			tex_width = image_desc.width;
			tex_height = image_desc.height;
			
			luset(tex, device->new_texture(TextureDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, 
				TextureUsageFlag::sampled_texture | TextureUsageFlag::copy_dest, image_desc.width, image_desc.height, 1, 1)));

			u64 size, row_pitch, slice_pitch;
			device->get_texture_data_placement_info(image_desc.width, image_desc.height, 1, Format::rgba8_unorm, &size, nullptr, &row_pitch, &slice_pitch);
			lulet(tex_staging, device->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::copy_source, size)));

			lulet(tex_staging_data, tex_staging->map(0, 0));
			memcpy_bitmap(tex_staging_data, image_data.data(), image_desc.width * 4, image_desc.height, row_pitch, image_desc.width * 4);
			tex_staging->unmap(0, size);

			u32 copy_queue_index = get_command_queue_index();
			{
				// Prefer a dedicated copy queue if present.
				u32 num_queues = device->get_num_command_queues();
				for (u32 i = 0; i < num_queues; ++i)
				{
					auto desc = device->get_command_queue_desc(i);
					if (desc.type == CommandQueueType::copy)
					{
						copy_queue_index = i;
						break;
					}
				}
			}
			lulet(upload_cmdbuf, device->new_command_buffer(copy_queue_index));
			upload_cmdbuf->set_context(CommandBufferContextType::copy);
			upload_cmdbuf->resource_barrier({
				{ tex_staging, BufferStateFlag::automatic, BufferStateFlag::copy_source, ResourceBarrierFlag::none}}, 
				{{ tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::copy_dest, ResourceBarrierFlag::discard_content }});
			upload_cmdbuf->copy_buffer_to_texture(tex, SubresourceIndex(0, 0), 0, 0, 0, tex_staging, 0, row_pitch, slice_pitch, image_desc.width, image_desc.height, 1);
			luexp(upload_cmdbuf->submit({}, {}, true));
			upload_cmdbuf->wait();
			luset(desc_set, device->new_descriptor_set(DescriptorSetDesc(desc_set_layout)));

			desc_set->update_descriptors(
				{
					DescriptorSetWrite::sampled_texture_view(0, TextureViewDesc::tex2d(tex)),
					DescriptorSetWrite::sampler(1, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp,
							TextureAddressMode::clamp, TextureAddressMode::clamp))
				});
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

	f32 width = tex_width;
	f32 height = tex_height;

	VertexData data[4] = {
		{ { -width / w,  height / h },{ 0.0f, 0.0f } },
		{ {  width / w,  height / h },{ 1.0f, 0.0f } },
		{ { -width / w, -height / h },{ 0.0f, 1.0f } },
		{ {  width / w, -height / h },{ 1.0f, 1.0f } }
	};

	void* mapped = vb->map(0, 0).get();
	memcpy(mapped, data, sizeof(data));
	vb->unmap(0, sizeof(data));

	auto cb = get_command_buffer();
	cb->set_context(CommandBufferContextType::graphics);
	cb->resource_barrier({},
		{
			{tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
			{get_back_buffer(), SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
		});
	RenderPassDesc desc;
	desc.color_attachments[0] = ColorAttachment(get_back_buffer(), LoadOp::clear, StoreOp::store, Color::black());
	cb->begin_render_pass(desc);
	cb->set_graphics_pipeline_state(pso);
	cb->set_graphics_shader_input_layout(shader_input_layout);
	IDescriptorSet* ds = desc_set.get();
	cb->set_graphics_descriptor_sets(0, { &ds, 1 });
	cb->set_vertex_buffers(0, {VertexBufferView(vb, 0, sizeof(VertexData) * 4, sizeof(VertexData))});
	cb->set_index_buffer({ ib, 0, 24, Format::r32_uint });
	cb->set_scissor_rect(RectI(0, 0, (i32)w, (i32)h));
	cb->set_viewport(Viewport(0.0f, 0.0f, (f32)w, (f32)h, 0.0f, 1.0f));
	cb->draw_indexed(6, 0, 0);
	cb->end_render_pass();
}

void resize(u32 width, u32 height)
{
}

void cleanup()
{
	desc_set_layout.reset();
	shader_input_layout.reset();
	desc_set.reset();
	pso.reset();
	tex.reset();
	vb.reset();
	ib.reset();
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