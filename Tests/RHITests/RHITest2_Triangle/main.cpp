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
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>

#include <TestTriangleVS.hpp>
#include <TestTrianglePS.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

Ref<RHI::IPipelineLayout> pipeline_layout;
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
            luset(pipeline_layout, get_main_device()->new_pipeline_layout(PipelineLayoutDesc({},
                PipelineLayoutFlag::allow_input_assembler_input_layout |
                PipelineLayoutFlag::deny_pixel_shader_access |
                PipelineLayoutFlag::deny_vertex_shader_access)));

            GraphicsPipelineStateDesc desc;
            const InputBindingDesc bindings[] = {
                InputBindingDesc(0, sizeof(VertexData), InputRate::per_vertex)
            };
            const InputAttributeDesc attributes[] = {
                InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rg32_float),
                InputAttributeDesc("COLOR", 0, 1, 0, 8, Format::rgba32_float)
            };
            desc.input_layout = InputLayoutDesc({bindings, 1}, {attributes, 2});
            desc.pipeline_layout = pipeline_layout;
            desc.vs = LUNA_GET_SHADER_DATA(TestTriangleVS);
            desc.ps = LUNA_GET_SHADER_DATA(TestTrianglePS);
            desc.rasterizer_state.depth_clip_enable = false;
            desc.depth_stencil_state = DepthStencilDesc(false, false);
            desc.num_color_attachments = 1;
            desc.color_formats[0] = Format::bgra8_unorm;

            luset(pso, get_main_device()->new_graphics_pipeline_state(desc));

            // prepare draw buffer. POSITION : COLOR
            VertexData data[3] = {
                { { 0.0f,  0.7f},{1.0f, 0.0f, 0.0f, 1.0f} },
                { { 0.7f, -0.7f},{0.0f, 1.0f, 0.0f, 1.0f} },
                { {-0.7f, -0.7f},{0.0f, 0.0f, 1.0f, 1.0f} }
            };

            luset(vb, get_main_device()->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(data))));
            void* mapped_data = nullptr;
            luexp(vb->map(0, 0, &mapped_data));
            memcpy(mapped_data, data, sizeof(data));
            vb->unmap(0, sizeof(data));
        }
    }
    lucatchret;
    return ok;
}

void draw()
{
    auto cb = get_command_buffer();
    cb->resource_barrier({}, {
            {get_back_buffer(), TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
        });
    RenderPassDesc desc;
    desc.color_attachments[0] = ColorAttachment(get_back_buffer(), LoadOp::clear, StoreOp::store, Color::yellow());
    cb->begin_render_pass(desc);
    cb->set_graphics_pipeline_state(pso);
    cb->set_graphics_pipeline_layout(pipeline_layout);
    IBuffer* vertex_buffer = vb;
    usize vb_offset = 0;
    cb->set_vertex_buffers(0, {VertexBufferView(vb, 0, sizeof(VertexData) * 3, sizeof(VertexData))});
    auto sz = get_window()->get_framebuffer_size();
    cb->set_scissor_rect(RectI(0, 0, (i32)sz.x, (i32)sz.y));
    cb->set_viewport(Viewport(0, 0, (f32)sz.x, (f32)sz.y, 0.0f, 1.0f));
    cb->draw(3, 0);
    cb->end_render_pass();
}

void resize(u32 width, u32 height)
{
}

void cleanup()
{
    pipeline_layout.reset();
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
    lupanic_if_failed(add_modules({module_rhi_test_bed(), module_shader_compiler()}));
    auto r = init_modules();
    if (failed(r))
    {
        log_error("%s", explain(r.errcode()));
    }
    else run_app();
    Luna::close();
    return 0;
}
