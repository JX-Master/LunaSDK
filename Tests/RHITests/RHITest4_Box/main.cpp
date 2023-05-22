/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2023/2/24
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Runtime/Runtime.hpp>
#include <Runtime/Module.hpp>
#include <Runtime/Debug.hpp>
#include <Runtime/Math/Color.hpp>
#include <ShaderCompiler/ShaderCompiler.hpp>
#include <Runtime/Log.hpp>
#include <RHI/ShaderCompileHelper.hpp>
#include <Runtime/Math/Matrix.hpp>
#include <Image/Image.hpp>
#include <Runtime/File.hpp>
#include <Runtime/Math/Transform.hpp>

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
Ref<RHI::IShaderInputLayout> slayout;
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
            {DescriptorType::sampled_texture_view, 1, 1, ShaderVisibilityFlag::pixel},
            {DescriptorType::sampler, 2, 1, ShaderVisibilityFlag::pixel}
        })));
        luset(desc_set, dev->new_descriptor_set(DescriptorSetDesc(dlayout)));

        const char vs_shader_code[] = R"(
            cbuffer vertexBuffer : register(b0)
            {
                float4x4 world_to_proj;
            };
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

        luset(slayout, dev->new_shader_input_layout(ShaderInputLayoutDesc({&dl, 1}, 
            ShaderInputLayoutFlag::allow_input_assembler_input_layout)));
        GraphicsPipelineStateDesc ps_desc;
        ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
		ps_desc.sample_mask = U32_MAX;
		ps_desc.blend_state = BlendDesc({ 
            AttachmentBlendDesc(false, BlendFactor::src_alpha, BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
		ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
		ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
		ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
        ps_desc.input_layout = InputLayoutDesc({
                {
                    InputBindingDesc(0, sizeof(Vertex), InputRate::per_vertex)
                },
                {
                    InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rgb32_float),
                    InputAttributeDesc("TEXCOORD", 0, 1, 0, 12, Format::rg32_float),
                } });
		ps_desc.vs = vs.cspan();
		ps_desc.ps = ps.cspan();
		ps_desc.shader_input_layout = slayout;
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

        luset(vb, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(vertices))));
        luset(ib, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::index_buffer, sizeof(indices))));
        luset(file_tex, dev->new_texture(MemoryType::local, TextureDesc::tex2d(Format::rgba8_unorm,
            TextureUsageFlag::sampled_texture | TextureUsageFlag::copy_dest, image_desc.width, image_desc.height, 1, 1)));

        lulet(mapped, vb->map(0, 0));
        memcpy(mapped, vertices, sizeof(vertices));
        vb->unmap(0, sizeof(vertices));
        luset(mapped, ib->map(0, 0));
        memcpy(mapped, indices, sizeof(indices));
        ib->unmap(0, sizeof(indices));

        u64 size, row_pitch, slice_pitch;
        dev->get_texture_data_placement_info(image_desc.width, image_desc.height, 1, Format::rgba8_unorm, &size, nullptr, &row_pitch, &slice_pitch);
        lulet(tex_staging, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::copy_source, size)));

        lulet(tex_staging_data, tex_staging->map(0, 0));
        memcpy_bitmap(tex_staging_data, image_data.data(), image_desc.width * 4, image_desc.height, row_pitch, image_desc.width * 4);
        tex_staging->unmap(0, size);
        
        u32 copy_queue_index = get_command_queue_index();
        {
            // Prefer a dedicated copy queue if present.
            u32 num_queues = dev->get_num_command_queues();
            for (u32 i = 0; i < num_queues; ++i)
            {
                auto desc = dev->get_command_queue_desc(i);
                if (desc.type == CommandQueueType::copy)
                {
                    copy_queue_index = i;
                    break;
                }
            }
        }
        lulet(upload_cmdbuf, dev->new_command_buffer(copy_queue_index));
        upload_cmdbuf->set_context(CommandBufferContextType::copy);
        upload_cmdbuf->resource_barrier({
            { tex_staging, BufferStateFlag::automatic, BufferStateFlag::copy_source, ResourceBarrierFlag::none} },
            { { file_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::copy_dest, ResourceBarrierFlag::discard_content } });
        upload_cmdbuf->copy_buffer_to_texture(file_tex, SubresourceIndex(0, 0), 0, 0, 0, tex_staging, 0, 
            image_desc.width* Image::pixel_size(image_desc.format), image_desc.width* image_desc.height* Image::pixel_size(image_desc.format),
            image_desc.width, image_desc.height, 1);
        luexp(upload_cmdbuf->submit({}, {}, true));
        upload_cmdbuf->wait();

        desc_set->update_descriptors(
            {
                WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb)),
                WriteDescriptorSet::sampled_texture_view(1, TextureViewDesc::tex2d(file_tex)),
                WriteDescriptorSet::sampler(2, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp,
                        TextureAddressMode::clamp, TextureAddressMode::clamp))
            });
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
        camera_mat = mul(camera_mat, ProjectionMatrix::make_perspective_fov(PI / 3.0f, (f32)window_sz.x / (f32)window_sz.y, 0.001f, 100.0f));
        lulet(camera_mapped, cb->map(0, 0));
        memcpy(camera_mapped, &camera_mat, sizeof(Float4x4));
        cb->unmap(0, sizeof(Float4x4));

        using namespace RHI;

		auto cmdbuf = get_command_buffer();
        cmdbuf->set_context(CommandBufferContextType::graphics);
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
        desc.depth_stencil_attachment = DepthStencilAttachment(depth_tex, LoadOp::clear, StoreOp::store, 1.0f);
        cmdbuf->begin_render_pass(desc);
        cmdbuf->set_graphics_shader_input_layout(slayout);
        cmdbuf->set_graphics_pipeline_state(pso);
        cmdbuf->set_graphics_descriptor_set(0, desc_set);
        auto sz = vb->get_desc().size;
        cmdbuf->set_vertex_buffers(0, {VertexBufferView(vb, 0, sz, sizeof(Vertex))});
        sz = ib->get_desc().size;
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
	slayout.reset();
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