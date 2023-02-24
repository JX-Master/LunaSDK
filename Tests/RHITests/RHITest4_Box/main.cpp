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
Ref<RHI::IResource> depth_tex;
Ref<RHI::IRenderTargetView> rtv;
Ref<RHI::IDepthStencilView> dsv;
Ref<RHI::IResource> vb;
Ref<RHI::IResource> ib;
Ref<RHI::IResource> cb;
Ref<RHI::IResource> file_tex;
f32 camera_rotation = 0.0f;

RV start()
{
	lutry
	{
		auto dev = RHI::get_main_device();

		using namespace RHI;
        luset(dlayout, dev->new_descriptor_set_layout(DescriptorSetLayoutDesc({
            {DescriptorType::cbv, 0, 1, ShaderVisibility::vertex},
            {DescriptorType::srv, 1, 1, ShaderVisibility::pixel},
            {DescriptorType::sampler, 2, 1, ShaderVisibility::pixel}
        })));
        luset(desc_set, dev->new_descriptor_set(DescriptorSetDesc(dlayout)));

        const char vs_shader_code[] = R"(
        cbuffer vertexBuffer : register(b0)
        {
            float4x4 world_to_proj;
        };
        struct VS_INPUT
        {
            float3 position : POSITION;
            float2 texcoord : TEXCOORD;
        };
        struct PS_INPUT
        {
            float4 position : SV_POSITION;
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
            float4 position : SV_POSITION;
            float2 texcoord : TEXCOORD;
        };
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
		compiler->set_shader_model(5, 0);
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
		compiler->set_shader_model(5, 0);
		compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
		luexp(compiler->compile());
		auto ps_data = compiler->get_output();
		Blob ps(ps_data.data(), ps_data.size());

        luset(slayout, dev->new_shader_input_layout(ShaderInputLayoutDesc({dlayout}, 
            ShaderInputLayoutFlag::allow_input_assembler_input_layout |
            ShaderInputLayoutFlag::deny_hull_shader_access |
            ShaderInputLayoutFlag::deny_domain_shader_access |
            ShaderInputLayoutFlag::deny_geometry_shader_access)));
        GraphicPipelineStateDesc ps_desc;
		ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
		ps_desc.sample_mask = U32_MAX;
		ps_desc.sample_quality = 0;
		ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(false, false, BlendFactor::src_alpha,
			BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
		ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
		ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
		ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
		ps_desc.input_layout = InputLayoutDesc({
            {"POSITION", 0, Format::rgb32_float},
            {"TEXCOORD", 0, Format::rg32_float},
        });
		ps_desc.vs = vs.cspan();
		ps_desc.ps = ps.cspan();
		ps_desc.shader_input_layout = slayout;
		ps_desc.num_render_targets = 1;
		ps_desc.rtv_formats[0] = Format::rgba8_unorm;
		ps_desc.dsv_format = Format::d32_float;
        luset(pso, dev->new_graphic_pipeline_state(ps_desc));
        
        auto window_size = get_window()->get_framebuffer_size();
        luset(depth_tex, dev->new_resource(ResourceDesc::tex2d(ResourceHeapType::local, Format::d32_float, 
            ResourceUsageFlag::depth_stencil, window_size.x, window_size.y, 1, 1)));
        luset(rtv, dev->new_render_target_view(get_back_buffer()));
        luset(dsv, dev->new_depth_stencil_view(depth_tex));

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
        luset(vb, dev->new_resource(ResourceDesc::buffer(ResourceHeapType::shared_upload, ResourceUsageFlag::vertex_buffer, sizeof(vertices))));
        luset(ib, dev->new_resource(ResourceDesc::buffer(ResourceHeapType::shared_upload, ResourceUsageFlag::index_buffer, sizeof(indices))));
        void* mapped = nullptr;
        luexp(vb->map_subresource(0, false, &mapped));
        memcpy(mapped, vertices, sizeof(vertices));
        vb->unmap_subresource(0, true);
        luexp(ib->map_subresource(0, false, &mapped));
        memcpy(mapped, indices, sizeof(indices));
        ib->unmap_subresource(0, true);
        auto cb_align = dev->get_constant_buffer_data_alignment();
        luset(cb, dev->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(Float4x4), cb_align))));

        lulet(image_file, open_file("Luna.png", FileOpenFlag::read, FileCreationMode::open_existing));
        lulet(image_file_data, load_file_data(image_file));
        Image::ImageDesc image_desc;
        lulet(image_data, Image::read_image_file(image_file_data.data(), image_file_data.size(), Image::ImagePixelFormat::rgba8_unorm, image_desc));

        luset(file_tex, dev->new_resource(ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, 
            ResourceUsageFlag::shader_resource, image_desc.width, image_desc.height, 1, 1)));
        luexp(file_tex->map_subresource(0, false));
        luexp(file_tex->write_subresource(0, image_data.data(), 
            image_desc.width * Image::pixel_size(image_desc.format), 
            image_desc.width * image_desc.height * Image::pixel_size(image_desc.format), BoxU(0, 0, 0, image_desc.width, image_desc.height, 1)));
        file_tex->unmap_subresource(0, true);

        desc_set->set_cbv(0, cb, ConstantBufferViewDesc(0, align_upper(sizeof(Float4x4), cb_align)));
        desc_set->set_srv(1, file_tex);
        desc_set->set_sampler(2, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::clamp,
				TextureAddressMode::clamp, TextureAddressMode::clamp, 0.0f, 1, ComparisonFunc::always, 
                Float4U(0, 0, 0, 0), 0.0f, 0.0f));
		
	}
	lucatchret;
	return ok;
}

void draw()
{
	lutry
    {
        camera_rotation += 1.0f;
        Float3 camera_pos(cosf(camera_rotation / 180.0f * PI) * 2.0f, 1.0f, sinf(camera_rotation / 180.0f * PI) * 2.0f);
        Float4x4 camera_mat = AffineMatrix::make_look_at(camera_pos, Float3(0, 0, 0), Float3(0, 1, 0));
        auto window_sz = get_window()->get_framebuffer_size();
        camera_mat = mul(camera_mat, ProjectionMatrix::make_perspective_fov(PI / 3.0f, (f32)window_sz.x / (f32)window_sz.y, 0.001f, 100.0f));
        void* camera_mapped;
        luexp(cb->map_subresource(0, false, &camera_mapped));
        memcpy(camera_mapped, &camera_mat, sizeof(Float4x4));
        cb->unmap_subresource(0, true);

        using namespace RHI;

		auto cmdbuf = get_command_buffer();

        cmdbuf->resource_barriers({
            ResourceBarrierDesc::as_transition(cb, ResourceState::vertex_and_constant_buffer),
            ResourceBarrierDesc::as_transition(vb, ResourceState::vertex_and_constant_buffer),
            ResourceBarrierDesc::as_transition(ib, ResourceState::index_buffer),
            ResourceBarrierDesc::as_transition(file_tex, ResourceState::shader_resource_pixel),
            ResourceBarrierDesc::as_transition(get_back_buffer(), ResourceState::render_target),
            ResourceBarrierDesc::as_transition(depth_tex, ResourceState::depth_stencil_write)
        });

        RenderPassDesc desc;
        desc.rtvs[0] = rtv;
        desc.rt_load_ops[0] = LoadOp::clear;
        desc.rt_store_ops[0] = StoreOp::store;
        desc.rt_clear_values[0] = {0, 0, 0, 0};
        desc.dsv = dsv;
        desc.depth_load_op = LoadOp::clear;
        desc.depth_store_op = StoreOp::store;
        desc.depth_clear_value = 1.0f;
        desc.stencil_load_op = LoadOp::dont_care;
        desc.stencil_store_op = StoreOp::dont_care;
        cmdbuf->begin_render_pass(desc);
        cmdbuf->set_graphic_shader_input_layout(slayout);
        cmdbuf->set_pipeline_state(pso);
        cmdbuf->set_graphic_descriptor_set(0, desc_set);
        cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
        auto sz = vb->get_desc().width_or_buffer_size;
        cmdbuf->set_vertex_buffers(0, {VertexBufferViewDesc(vb, 0, sz, sizeof(Vertex))});
        sz = ib->get_desc().width_or_buffer_size;
        cmdbuf->set_index_buffer(ib, 0, sz, Format::r32_uint);
        cmdbuf->set_scissor_rect(RectI(0, 0, (i32)window_sz.x, (i32)window_sz.y));
        cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)window_sz.x, (f32)window_sz.y, 0.0f, 1.0f));
        cmdbuf->draw_indexed(36, 0, 0);
        cmdbuf->end_render_pass();

        luexp(cmdbuf->submit());
		cmdbuf->wait();
        luexp(cmdbuf->reset());
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
        luset(depth_tex, dev->new_resource(ResourceDesc::tex2d(ResourceHeapType::local, Format::d32_float, 
            ResourceUsageFlag::depth_stencil, width, height, 1, 1)));
        luset(rtv, dev->new_render_target_view(get_back_buffer()));
        luset(dsv, dev->new_depth_stencil_view(depth_tex));
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
	rtv.reset();
	dsv.reset();
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