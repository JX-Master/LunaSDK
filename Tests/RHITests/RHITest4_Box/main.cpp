/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2023/2/24
*/
//#define LUNA_DISABLE_SIMD
#include "../RHITestBed/RHITestBed.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Debug.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/RHI/Utility.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

struct Vertex
{
    Float3U position;
    Float2U texcoord;
};

Ref<RHI::IDescriptorSetLayout> dlayout;
Ref<RHI::IDescriptorSet> desc_set;
Ref<RHI::IPipelineLayout> playout;
Ref<RHI::IPipelineState> pso;
Ref<RHI::ITexture> depth_tex;
Ref<RHI::IBuffer> vb;
Ref<RHI::IBuffer> ib;
Ref<RHI::IBuffer> cb;
Ref<RHI::ITexture> file_tex;
f32 camera_rotation = 0.0f;

RV start()
{
	lutry
	{
		auto dev = RHI::get_main_device();

		using namespace RHI;
        luset(dlayout, dev->new_descriptor_set_layout(DescriptorSetLayoutDesc({
            {DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::vertex},
            {DescriptorType::read_texture_view, 1, 1, ShaderVisibilityFlag::pixel},
            {DescriptorType::sampler, 2, 1, ShaderVisibilityFlag::pixel}
        })));
        luset(desc_set, dev->new_descriptor_set(DescriptorSetDesc(dlayout)));

        const char vs_shader_code[] = R"(
            cbuffer vertexBuffer : register(b0)
            {
                float4x4 world_to_proj;
            };
            Texture2D tex : register(t1);
            SamplerState tex_sampler : register(s2);
            struct VS_INPUT
            {
                [[vk::location(0)]]
                float3 position : POSITION;
                [[vk::location(1)]]
                float2 texcoord : TEXCOORD;
            };
            struct PS_INPUT
            {
                [[vk::location(0)]]
                float4 position : SV_POSITION;
                [[vk::location(1)]]
                float2 texcoord : TEXCOORD;
            };
            PS_INPUT main(VS_INPUT input)
            {
                PS_INPUT output;
                output.position = mul(world_to_proj, float4(input.position, 1.0f));
                output.texcoord = input.texcoord;
                return output;
            })";
        const char ps_shader_code[] = R"(
            cbuffer vertexBuffer : register(b0)
            {
                float4x4 world_to_proj;
            };
            Texture2D tex : register(t1);
            SamplerState tex_sampler : register(s2);
            struct PS_INPUT
            {
                [[vk::location(0)]]
                float4 position : SV_POSITION;
                [[vk::location(1)]]
                float2 texcoord : TEXCOORD;
            };
            [[vk::location(0)]]
            float4 main(PS_INPUT input) : SV_Target
            {
                return float4(tex.Sample(tex_sampler, input.texcoord));
            })";
        auto compiler = ShaderCompiler::new_compiler();
		compiler->set_source({ vs_shader_code, strlen(vs_shader_code)});
		compiler->set_source_name("DemoAppVS");
		compiler->set_entry_point("main");
		compiler->set_target_format(RHI::get_current_platform_shader_target_format());
		compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
		compiler->set_shader_model(6, 0);
		compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
		luexp(compiler->compile());
		auto vs_data = compiler->get_output();
		Blob vs(vs_data.data(), vs_data.size());

        compiler->reset();
		compiler->set_source({ ps_shader_code, strlen(ps_shader_code)});
		compiler->set_source_name("DemoAppPS");
		compiler->set_entry_point("main");
		compiler->set_target_format(RHI::get_current_platform_shader_target_format());
		compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
		compiler->set_shader_model(6, 0);
		compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
		luexp(compiler->compile());
		auto ps_data = compiler->get_output();
		Blob ps(ps_data.data(), ps_data.size());

        IDescriptorSetLayout* dl = dlayout;

        luset(playout, dev->new_pipeline_layout(PipelineLayoutDesc({&dl, 1}, 
            PipelineLayoutFlag::allow_input_assembler_input_layout)));
        GraphicsPipelineStateDesc ps_desc;
        ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
		ps_desc.sample_mask = U32_MAX;
		ps_desc.blend_state = BlendDesc({ 
            AttachmentBlendDesc(false, BlendFactor::src_alpha, BlendFactor::one_minus_src_alpha, BlendOp::add, BlendFactor::one_minus_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
		ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, false, true, false, false, false);
		ps_desc.depth_stencil_state = DepthStencilDesc(true, true, CompareFunction::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
		ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
        InputBindingDesc bindings[] = {InputBindingDesc(0, sizeof(Vertex), InputRate::per_vertex)};
        InputAttributeDesc attributes[] = {
            InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rgb32_float),
            InputAttributeDesc("TEXCOORD", 0, 1, 0, 12, Format::rg32_float)
        };
        ps_desc.input_layout = InputLayoutDesc({bindings, 1}, {attributes, 2});
		ps_desc.vs = vs.cspan();
		ps_desc.ps = ps.cspan();
		ps_desc.pipeline_layout = playout;
		ps_desc.num_color_attachments = 1;
		ps_desc.color_formats[0] = Format::bgra8_unorm;
		ps_desc.depth_stencil_format = Format::d32_float;
        luset(pso, dev->new_graphics_pipeline_state(ps_desc));
        
        auto window_size = get_window()->get_framebuffer_size();
        luset(depth_tex, dev->new_texture(MemoryType::local, TextureDesc::tex2d(Format::d32_float,
            TextureUsageFlag::depth_stencil_attachment, window_size.x, window_size.y, 1, 1)));

        Vertex vertices[] = {
            {{+0.5, -0.5, -0.5}, {0.0, 1.0}}, {{+0.5, +0.5, -0.5}, {0.0, 0.0}},
            {{+0.5, +0.5, +0.5}, {1.0, 0.0}}, {{+0.5, -0.5, +0.5}, {1.0, 1.0}},

            {{+0.5, -0.5, +0.5}, {0.0, 1.0}}, {{+0.5, +0.5, +0.5}, {0.0, 0.0}},
            {{-0.5, +0.5, +0.5}, {1.0, 0.0}}, {{-0.5, -0.5, +0.5}, {1.0, 1.0}},

            {{-0.5, -0.5, +0.5}, {0.0, 1.0}}, {{-0.5, +0.5, +0.5}, {0.0, 0.0}},
            {{-0.5, +0.5, -0.5}, {1.0, 0.0}}, {{-0.5, -0.5, -0.5}, {1.0, 1.0}},

            {{-0.5, -0.5, -0.5}, {0.0, 1.0}}, {{-0.5, +0.5, -0.5}, {0.0, 0.0}},
            {{+0.5, +0.5, -0.5}, {1.0, 0.0}}, {{+0.5, -0.5, -0.5}, {1.0, 1.0}},

            {{-0.5, +0.5, -0.5}, {0.0, 1.0}}, {{-0.5, +0.5, +0.5}, {0.0, 0.0}},
            {{+0.5, +0.5, +0.5}, {1.0, 0.0}}, {{+0.5, +0.5, -0.5}, {1.0, 1.0}},

            {{+0.5, -0.5, -0.5}, {0.0, 1.0}}, {{+0.5, -0.5, +0.5}, {0.0, 0.0}},
            {{-0.5, -0.5, +0.5}, {1.0, 0.0}}, {{-0.5, -0.5, -0.5}, {1.0, 1.0}}
        };
        u32 indices[] = {
            0, 1, 2, 0, 2, 3, 
            4, 5, 6, 4, 6, 7, 
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23
        };
        auto cb_align = dev->get_uniform_buffer_data_alignment();
        luset(cb, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(Float4x4), cb_align))));

        lulet(image_file, open_file("luna.png", FileOpenFlag::read, FileCreationMode::open_existing));
        lulet(image_file_data, load_file_data(image_file));
        Image::ImageDesc image_desc;
        lulet(image_data, Image::read_image_file(image_file_data.data(), image_file_data.size(), Image::ImagePixelFormat::rgba8_unorm, image_desc));

        luset(vb, dev->new_buffer(MemoryType::local, BufferDesc(BufferUsageFlag::vertex_buffer | BufferUsageFlag::copy_dest, sizeof(vertices))));
        luset(ib, dev->new_buffer(MemoryType::local, BufferDesc(BufferUsageFlag::index_buffer | BufferUsageFlag::copy_dest, sizeof(indices))));
        luset(file_tex, dev->new_texture(MemoryType::local, TextureDesc::tex2d(Format::rgba8_unorm,
            TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, image_desc.width, image_desc.height, 1, 1)));
        
        u32 copy_queue_index = get_command_queue_index();
        lulet(upload_cmdbuf, dev->new_command_buffer(copy_queue_index));
        luexp(copy_resource_data(upload_cmdbuf, {
                CopyResourceData::write_buffer(vb, 0, vertices, sizeof(vertices)),
                CopyResourceData::write_buffer(ib, 0, indices, sizeof(indices)),
				CopyResourceData::write_texture(file_tex, SubresourceIndex(0, 0), 0, 0, 0, 
					image_data.data(), image_desc.width * 4, image_desc.width * image_desc.height * 4, 
					image_desc.width, image_desc.height, 1)}));
        luexp(desc_set->update_descriptors(
            {
                WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb)),
                WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(file_tex)),
                WriteDescriptorSet::sampler(2, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp,
                        TextureAddressMode::clamp, TextureAddressMode::clamp))
            }));
	}
	lucatchret;
	return ok;
}

void draw()
{
	lutry
    {
        camera_rotation += 1.0f;
        Float3 camera_pos(cosf(camera_rotation / 180.0f * PI) * 3.0f, 1.0f, sinf(camera_rotation / 180.0f * PI) * 3.0f);
        Float4x4 camera_mat = AffineMatrix::make_look_at(camera_pos, Float3(0, 0, 0), Float3(0, 1, 0));
        auto window_sz = get_window()->get_framebuffer_size();
        camera_mat = mul(camera_mat, ProjectionMatrix::make_perspective_fov(PI / 3.0f, (f32)window_sz.x / (f32)window_sz.y, 1.0f, 4.0f));
        void* camera_mapped = nullptr;
        luexp(cb->map(0, 0, &camera_mapped));
        memcpy(camera_mapped, &camera_mat, sizeof(Float4x4));
        cb->unmap(0, sizeof(Float4x4));

        using namespace RHI;

		auto cmdbuf = get_command_buffer();
        cmdbuf->resource_barrier(
            {
                {cb, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_vs, ResourceBarrierFlag::none},
                {vb, BufferStateFlag::automatic, BufferStateFlag::vertex_buffer, ResourceBarrierFlag::none},
                {ib, BufferStateFlag::automatic, BufferStateFlag::index_buffer, ResourceBarrierFlag::none},
            },
            {
                {file_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
                {get_back_buffer(), SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::none},
                {depth_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::depth_stencil_attachment_write, ResourceBarrierFlag::none},
            }
        );
        RenderPassDesc desc;
        desc.color_attachments[0] = ColorAttachment(get_back_buffer(), LoadOp::clear, StoreOp::store, { 0, 0, 0, 0 });
        desc.depth_stencil_attachment = DepthStencilAttachment(depth_tex, false, LoadOp::clear, StoreOp::store, 1.0f);
        cmdbuf->begin_render_pass(desc);
        cmdbuf->set_graphics_pipeline_layout(playout);
        cmdbuf->set_graphics_pipeline_state(pso);
        cmdbuf->set_graphics_descriptor_set(0, desc_set);
        u32 sz = (u32)vb->get_desc().size;
        cmdbuf->set_vertex_buffers(0, {VertexBufferView(vb, 0, sz, sizeof(Vertex))});
        sz = (u32)ib->get_desc().size;
        cmdbuf->set_index_buffer({ ib, 0, 144, Format::r32_uint });
        cmdbuf->set_scissor_rect(RectI(0, 0, (i32)window_sz.x, (i32)window_sz.y));
        cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)window_sz.x, (f32)window_sz.y, 0.0f, 1.0f));
        cmdbuf->draw_indexed(36, 0, 0);
        cmdbuf->end_render_pass();
    }
    lucatch
	{
		lupanic();
	}
}

void resize(u32 width, u32 height)
{
	lutry
    {
        using namespace RHI;
        auto dev = get_main_device();
        luset(depth_tex, dev->new_texture(MemoryType::local, TextureDesc::tex2d(Format::d32_float,
            TextureUsageFlag::depth_stencil_attachment, width, height, 1, 1)));
    }
    lucatch
	{
		lupanic();
	}
}

void cleanup()
{
	dlayout.reset();
	desc_set.reset();
	playout.reset();
	pso.reset();
	depth_tex.reset();
	vb.reset();
	ib.reset();
	cb.reset();
	file_tex.reset();
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
